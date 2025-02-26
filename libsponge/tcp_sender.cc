#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity) {
        _retransmission_timeout=retx_timeout;
}

uint64_t TCPSender::bytes_in_flight() const {
    return _bytes_in_flight;
}

void TCPSender::fill_window() {
    // 接收端会给发送端一个window_size,根据这个window_size，如果窗口没有被填满。并且发送端有数据需要发送，就发送出去，即压入_segments_out队列中

    // 如果还没发送SYN段，则发送一个SYN段
    if(!syn_flag){
        TCPSegment seg;
        seg.header().syn=true;
        send_segment(seg);
        syn_flag = true;
        return ;
    }

    // 发送窗口
    size_t win=_window_size>0? _window_size:1;
    // 空余窗口
    size_t remain;
    // 还没有发过FIN段，且窗口还有空间
    while(!fin_flag){
        // 空闲空间=窗口大小-已经发送但还没收到确认
        remain=win-(_next_seqno - _recv_ackno);
        if(remain==0){
            break;
        }
        size_t size = min(TCPConfig::MAX_PAYLOAD_SIZE, remain);
        TCPSegment seg;
        // 填充内容
        std::stringstream ss;
        ss<<_stream.read(size);
        seg.payload()=Buffer(std::move(ss.str()));
        // 还能填充
        if(seg.length_in_sequence_space()<win && _stream.eof()){
            seg.header().fin=true;
            fin_flag=true;
        }
        // 如果是空的seg段
        if(seg.length_in_sequence_space()==0){
            return ;
        }
        send_segment(seg);
    }
    return ;
}

bool TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {

    size_t abs_ackno=unwrap(ackno,_isn,_recv_ackno);
    if(abs_ackno>_next_seqno){
        return false;
    }
    // 如果ACK合法
    _window_size=window_size;
    
    // 如果接收的abs_ackno已经确认过
    if(abs_ackno<=_recv_ackno){
        return true;
    }

    // 更新_recv_ackno
    _recv_ackno=abs_ackno;

    // 弹出所有确认号之前的数据
    while(!segments_unconfirmed.empty()){
        TCPSegment seg=segments_unconfirmed.front();
        // 如果有可以通过累积确认的数据段
        if(unwrap(seg.header().seqno,_isn,_recv_ackno) + seg.length_in_sequence_space()<=abs_ackno){
            segments_unconfirmed.pop();
            _bytes_in_flight-= seg.length_in_sequence_space();
        }
        // 如果队首不行，则弹出来
        else{
            break;
        }
    }
    
    // 有数据接收到了确认，现在可以窗口右移了
    fill_window();

    // 重置超时重传计时器和重传次数
    _retransmission_timeout=_initial_retransmission_timeout;
    _consecutive_retransmissions=0;

    // 如果还有未收到的，则重新启动
    if(!segments_unconfirmed.empty()){
        _timer_running=true;
        _timer=0;
    }
    return true;
}

void TCPSender::tick(const size_t ms_since_last_tick) {
    // 维护TCPSender的重传时间
    _timer+=ms_since_last_tick;
    // 如果时间超过了RTO，且有已经发送且没有被确认的，则重传
    if(_timer>=_retransmission_timeout && !segments_unconfirmed.empty()){
        // 第一个发送的肯定是最早出现超时的
        _segments_out.push(segments_unconfirmed.front());
        // 记录重传次数
        _consecutive_retransmissions++;
        // RTO翻倍
        _retransmission_timeout*=2;
        _timer_running=true;
        _timer = 0;
    }
    if(segments_unconfirmed.empty()){
        _timer_running=false;
    }
}


unsigned int TCPSender::consecutive_retransmissions() const {
    return _consecutive_retransmissions;
}

void TCPSender::send_empty_segment() {
    // 发送空的TCP段
    TCPSegment seg;
    seg.header().seqno=wrap(_next_seqno,_isn);
    _segments_out.push(seg);
    return ;
}

void TCPSender::send_empty_segment(WrappingInt32 seqno){
    TCPSegment seg;
    seg.header().seqno=seqno;
    // 加入发送队列
    _segments_out.push(seg);
    return ;
}

void TCPSender::send_segment(TCPSegment &seg){
    // 填写相对序号
    seg.header().seqno=wrap(_next_seqno,_isn);
    // 更新绝对序号
    _next_seqno+=seg.length_in_sequence_space();
    // 更新已经发送但没有确认的字节数
    _bytes_in_flight+=seg.length_in_sequence_space();
    segments_unconfirmed.push(seg);
    _segments_out.push(seg);
    // 如果重传计时器没有开始运行
    if(!_timer_running){
        _timer_running=true;
        _timer=0;
    }
}

