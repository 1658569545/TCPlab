#include "tcp_connection.hh"

#include <iostream>

// run make check

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

size_t TCPConnection::remaining_outbound_capacity() const { 
    return _sender.stream_in().remaining_capacity(); 
}

size_t TCPConnection::bytes_in_flight() const { 
    return _sender.bytes_in_flight(); 
}

size_t TCPConnection::unassembled_bytes() const { 
    return _receiver.unassembled_bytes(); 
}

size_t TCPConnection::time_since_last_segment_received() const { 
    return last_since_ack_time; 
}

void TCPConnection::segment_received(const TCPSegment &seg) { 
    // 接收TCP段的话，主要使用_receiver
    if(!_active){
        return ;
    }
    last_since_ack_time=0;

    // 如果处于SYN_SENT状态（即完成了第一个握手），则忽略掉带有数据的ACK段，因为需要的是不带数据的ACK段（即需要第二次握手）
    if(in_syn_sent() && seg.header().ack && seg.payload().size()>0){
        return ;
    }
    bool send_empty = false;
    // 处理ACK段
    if(_sender.next_seqno_absolute()>0 && seg.header().ack){
        // 不是新的确认，因此是重复的，需要重复发送ACK
        if(!_sender.ack_received(seg.header().ackno,seg.header().win)){
            send_empty=true;
        }
    }
    bool recv_flag=_receiver.segment_received(seg);
    // 接收的数据段失序，因此需要_sender发送ACK
    if(!recv_flag){
        send_empty=true;
    }

    // 如果接收的是SYN段且下一个要发送的是0号序列（代表没有发送过数据，因此要发起连接）
    if(seg.header().syn && _sender.next_seqno_absolute()==0){
        connect();
        return ;
    }

    // 接收的是rst段
    if(seg.header().rst){
        // 没有ack的RST段应该在SYN_SENT状态中被忽略
        if(in_syn_sent() && !seg.header().ack){
            return ;
        }
        unclean_shutdown(false);
        return ;
    }

    // 有效的数据段
    if(seg.length_in_sequence_space()>0){
        send_empty=true;
    }
    
    // 发送空的ACK数据段
    if(send_empty){
        // 接收方的序列号有效，即>0，且发送方的发送队列为空（保证没有重复的ACK）
        if(_receiver.ackno().has_value() && _sender.segments_out().empty()){
            _sender.send_empty_segment();
        }
    }
    push_segments_out();
}

bool TCPConnection::active() const { 
    return _active;
}

size_t TCPConnection::write(const std::string &data) {
    size_t res = _sender.stream_in().write(data);
    // 发送
    push_segments_out();
    return res;
}

void TCPConnection::tick(const size_t ms_since_last_tick) { 
    // 如果连接不存在就返回
    if(!_active){
        return ;
    }
    last_since_ack_time+=ms_since_last_tick;
    _sender.tick(ms_since_last_tick);
    // 多次超时需要终止连接，即此时可以判断对方不存在了
    if(_sender.consecutive_retransmissions() > TCPConfig::MAX_RETX_ATTEMPTS){
        unclean_shutdown(true);
    } 
    // 保证每次定时器被调用的时候，都能推送数据，因为_sender的tick()函数会将超时的重新加入_sender的输出队列
    // 因此需要调用此函数来保证重传的数据也能正确发送
    push_segments_out();
}

void TCPConnection::end_input_stream() {
    _sender.stream_in().end_input();
    // 确保FIN等数据段能够即使发送出去
    push_segments_out();
}

void TCPConnection::connect() {
    // 发送SYN段
    push_segments_out(true);
}

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            std::cerr << "Warning: Unclean shutdown of TCPConnection\n";
            unclean_shutdown(true);
        }
    } catch (const std::exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}

bool TCPConnection::in_syn_sent()  {
    // syn_sent是已经发送了SYN段但是还没有接收的状态

    // SYN会消耗一个序列号，因此要发送的下一个绝对序列号应该大于0，此时要发送的下一个绝对序列号就是1,并且发送出去但是还未确认字节数就是1
    return _sender.next_seqno_absolute()>0 && _sender.bytes_in_flight()==_sender.next_seqno_absolute();
}

bool TCPConnection::in_syn_recv() {
    // syn_recv是指远程对等方已经发送了对本地方SYN段的确认，从而等待接收本地方ACK的状态
    // 如何判断处于syn_recv状态
    //  首先是远方的ackno()函数肯定会有值，因为接收方的_base是期待的下一个到来的相对序列号
    //  而ackno()是返回该值的绝对值，又因为接收方接收到了相对序号为0的SYN段，因此_base将会是1
    //  因此，有了第一个判断条件 _receiver.ackno().has_value();
    // 其次，接收方的入站流也不会结束，因为还要接收数据

    return _receiver.ackno().has_value() && !_receiver.stream_out().input_ended();
}

void TCPConnection::unclean_shutdown(bool sent_rst){
    _receiver.stream_out().set_error();
    _sender.stream_in().set_error();
    _active=false;
    if(sent_rst){
        _need_send_rst=true;
        if(_sender.segments_out().empty()){
            _sender.send_empty_segment();
        }
        push_segments_out();
    }
}

bool TCPConnection::clean_shutdown(){
    // 入站流在出站流达到EOF之前就结束了
    if(_receiver.stream_out().input_ended() && !(_sender.stream_in().eof())){
        _linger_after_streams_finish=false;
    }
    // 出站流结束、出站流全部确认、入站流完全组装并且已经结束
    if(_sender.stream_in().eof() && _sender.bytes_in_flight()==0 && _receiver.stream_out().input_ended()){
        if(_linger_after_streams_finish == false || time_since_last_segment_received() >= 10 * _cfg.rt_timeout){
            _active = false;    
        }
    }
    return !_active;

}

bool TCPConnection::push_segments_out(bool send_syn){
    // 处于syn_recv状态时，需要发送SYN_ACK段
    _sender.fill_window(send_syn || in_syn_recv());

    TCPSegment seg;

    // 将_sender的发送队列中的数据放入 TCPConnection的输出队列中
    while(!_sender.segments_out().empty()){
        seg=_sender.segments_out().front();
        _sender.segments_out().pop();
        // 如果接收方收到过SYN段，现在开始发送
        if(_receiver.ackno().has_value()){
            seg.header().ack=true;
            // 根据接收到的ack来设置要发送的数据段的序列号
            seg.header().ackno=_receiver.ackno().value();
            seg.header().win=_receiver.window_size();
        }
        if(_need_send_rst){
            _need_send_rst=false;
            seg.header().rst=true;
        }
        _segments_out.push(seg);
    }
    clean_shutdown();
    return true;
}