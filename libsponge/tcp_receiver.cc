#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

bool TCPReceiver::segment_received(const TCPSegment &seg) {
    TCPSegment temp=seg;
    return true;
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    return std::nullopt;
}

size_t TCPReceiver::window_size() const {
    return 0;
}
