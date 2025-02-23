#ifndef SPONGE_LIBSPONGE_TCP_SEGMENT_HH
#define SPONGE_LIBSPONGE_TCP_SEGMENT_HH

#include "buffer.hh"
#include "tcp_header.hh"

#include <cstdint>

//! \brief [TCP](\ref rfc::rfc793) segment
class TCPSegment {
  private:
    TCPHeader _header{};
    Buffer _payload{};

  public:
    //! \brief Parse the segment from a string
    ParseResult parse(const Buffer buffer, const uint32_t datagram_layer_checksum = 0);

    //! \brief Serialize the segment to a string
    BufferList serialize(const uint32_t datagram_layer_checksum = 0) const;

    //! \name Accessors
    //!@{
    const TCPHeader &header() const { return _header; }
    TCPHeader &header() { return _header; }

    const Buffer &payload() const { return _payload; }
    Buffer &payload() { return _payload; }
    //!@}

    /**
     * @brief 片段在序列空间中的长度
     * @attention 如果设置了SYN，则等于有效载荷长度加一个字节，如果设置了FIN，则等于一个字节
     */
    size_t length_in_sequence_space() const;
};

#endif  // SPONGE_LIBSPONGE_TCP_SEGMENT_HH
