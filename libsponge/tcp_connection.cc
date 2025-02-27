#include "tcp_connection.hh"

#include <iostream>

// run make check

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

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

    // 处理ACK段
    

}

bool TCPConnection::active() const { 
    return _active;
}

size_t TCPConnection::write(const string &data) {
    if(data.size()==0){
        return 0;
    }
    size_t res = _sender.stream_in().write(data);
    return res;
}

void TCPConnection::tick(const size_t ms_since_last_tick) { 
    last_since_ack_time+=ms_since_last_tick;
}

void TCPConnection::end_input_stream() {

}

void TCPConnection::connect() {

}

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";

            // Your code here: need to send a RST segment to the peer
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}

bool TCPConnection::in_syn_sent() const {
    // syn_sent是已经发送了SYN段但是还没有接收的状态

    // SYN会消耗一个序列号，因此要发送的下一个绝对序列号应该大于0，此时要发送的下一个绝对序列号就是1,并且发送出去但是还未确认字节数就是1
    return _sender.next_seqno_absolute()>0 && _sender.bytes_in_flight()==_sender.next_seqno_absolute();
}

bool TCPConnection::in_syn_recv() const{
    // syn_recv是指远程对等方已经发送了对本地方SYN段的确认，从而等待接收本地方ACK的状态
    // 如何判断处于syn_recv状态
    //  首先是远方的ackno()函数肯定会有值，因为接收方的_base是期待的下一个到来的相对序列号
    //  而ackno()是返回该值的绝对值，又因为接收方接收到了相对序号为0的SYN段，因此_base将会是1
    //  因此，有了第一个判断条件 _receiver.ackno().has_value();
    // 其次，接收方的入站流也不会结束，因为还要接收数据

    return _receiver.ackno().has_value() && _receiver.stream_out().input_ended();
}
