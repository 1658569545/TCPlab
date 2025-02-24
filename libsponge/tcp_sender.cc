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

uint64_t TCPSender::bytes_in_flight() const { return {}; }

void TCPSender::fill_window() {}

bool TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    DUMMY_CODE(ackno, window_size);
    return {};
}

void TCPSender::tick(const size_t ms_since_last_tick) {

}

unsigned int TCPSender::consecutive_retransmissions() const {

}

void TCPSender::send_empty_segment() {

}

