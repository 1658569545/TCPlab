#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity) {}

uint64_t TCPSender::bytes_in_flight() const {
    return _bytes_in_flight;
}

void TCPSender::fill_window() {
    // 将ByteStream中的数据包装成数据段，并填入窗口中
    
    // 首先是检查SYN段
    if(!syn_flag){
        if(send_syn){

        }
    }

}

bool TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {

}

void TCPSender::tick(const size_t ms_since_last_tick) {

}

unsigned int TCPSender::consecutive_retransmissions() const {
    return _consecutive_retransmissions;
}

void TCPSender::send_empty_segment() {
    // 发送空的TCP端
    TCPSegment seg;
    seg.header().seqno=wrap(_next_seqno,_isn);
    _segments_out.push(seg);
}

