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
        rto=retx_timeout;
}

uint64_t TCPSender::bytes_in_flight() const {
    return bytes;
}

void TCPSender::fill_window() {
    // 先判断SYN
    if(!syn_flag){
        // 发送SYN段
        TCPSegment seg;
        seg.header().syn=true;
        send_segment(seg);
        syn_flag=true;
        return ;
    }
    

    // 先获取窗口大小
    size_t win=_window_size > 0 ? _window_size : 1;
    // 空余位置
    size_t remain;
    while(!fin_flag){
        // 空闲=窗口-已经发送的
        // 已经发送的=要发送的-已经收到确认的
        remain=win-(_next_seqno-recv_seqno);
        if(remain==0){
            break;
        }
        // 实际上的窗口空闲大小
        size_t size=min(TCPConfig::MAX_PAYLOAD_SIZE,remain);
        // 读取内容
        std::stringstream ss;
        ss<<_stream.read(size);
        TCPSegment seg;
        seg.payload()=Buffer(std::move(ss.str()));
        // 到达了末尾，且还有空闲空间，则加一个fin标志，fin标志也会消耗一个序列
        if(_stream.eof() && seg.length_in_sequence_space()<win){
            seg.header().fin=true;
            fin_flag=true;
        }
        // 如果长度为0,则代表输出流是空的了（但不代表结束了，可能后面还会有字节流到来）
        if(seg.length_in_sequence_space()==0){
            return ;
        }
        send_segment(seg);
    }
    return ;
}

bool TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    // 先将收到的ackno转换为绝对序列号
    size_t abs_ackno=unwrap(ackno,_isn,recv_seqno);
    // 更新窗口大小
    _window_size = window_size;

    // 大于下一个要发送的绝对序列号，肯定不行
    if(abs_ackno>_next_seqno){
        return false;
    }

    // 已经接收过了
    if(abs_ackno<=recv_seqno){
        return true;
    }

    // 排除掉以上两种之后，更新确认号
    recv_seqno=abs_ackno;

    // 更新segment_outstading队列
    while(!_segments_outstanding.empty()){
        TCPSegment seg=_segments_outstanding.front();
        // 判断TCP段是否被确认-----累积确认
        // 小笔记：
        // 已知ack n是确认n-1都已经到达，为什么这里可以等于
        // 因为：例如ack=100,seqno=0,length=100,很明显tcp段的序号是0-99，但是0+100=100,因此可以=
        if(unwrap(seg.header().seqno,_isn,recv_seqno) + seg.length_in_sequence_space()<=abs_ackno){
            _segments_outstanding.pop();
            bytes-=seg.length_in_sequence_space();
        }
        else{
            break;
        }
    }

    // 现在接收到了确认之后，就需要窗口往右边移动，因此，对窗口进行填充，并进行发送
    fill_window();

    // 发送之后，便重新启动计时和重传次数
    rto=_initial_retransmission_timeout;
    retransmissions=0;
    // 很明显，只有有发送出去的TCP段，才会开始计时
    if(!segments_out().empty()){
        timer=0;
        timer_running=true;
    }
    return true;

}

void TCPSender::tick(const size_t ms_since_last_tick) {
    // 超时重传的是全局累积的

    // 时间累积
    if(timer_running==true)    timer+=ms_since_last_tick;

    // 如果超时了,则超时重传
    if(timer>=rto && !_segments_outstanding.empty() ){
        _segments_out.push(_segments_outstanding.front());
        retransmissions++;
        // 重传了，就要重新启动计时器
        rto*=2;
        timer_running = true;
        timer = 0;
    }
    if(_segments_outstanding.empty() ){
        timer_running=false;
    }
}


unsigned int TCPSender::consecutive_retransmissions() const {
    return retransmissions;
}

void TCPSender::send_empty_segment(){
    TCPSegment seg;
    seg.header().seqno=wrap(_next_seqno,_isn);
    // 加入发送队列
    _segments_out.push(seg);
    return ;
}

void TCPSender::send_segment(TCPSegment &seg){
    seg.header().seqno=wrap(_next_seqno,_isn);
    
    // 更新相关数据
    _next_seqno+=seg.length_in_sequence_space();
    bytes+=seg.length_in_sequence_space();
    _segments_out.push(seg);
    _segments_outstanding.push(seg);
    // 启动重传计时器
    if(!timer_running){
        timer_running=true;
        timer=0;
    }
    return ;
}