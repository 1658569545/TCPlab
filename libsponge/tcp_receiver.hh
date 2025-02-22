#ifndef SPONGE_LIBSPONGE_TCP_RECEIVER_HH
#define SPONGE_LIBSPONGE_TCP_RECEIVER_HH

#include "byte_stream.hh"
#include "stream_reassembler.hh"
#include "tcp_segment.hh"
#include "wrapping_integers.hh"

#include <optional>

/**
 * @brief TCP实现的“接收方”部分。
 * @attention 接收和重新组装段成字节流，并计算确认号和窗口大小，以通告回远程TCPSender。
 */
class TCPReceiver {
    /// @brief 用于重新组装字节的数据结构。
    StreamReassembler _reassembler;

    /// @brief 存储的最大字节数。
    size_t _capacity;

  public:
    /**
     * @brief 构造函数
     * @param[in] capacity 接收器所能接收的最大字节数
     * @attention 会调用StreamReassembler的构造函数来初始化_reassembler
     */
    TCPReceiver(const size_t capacity) : _reassembler(capacity), _capacity(capacity) {}

    /**
     * @brief 发送确认序号给发送方
     * @return 如果未接收到SYN，则返回空
     * @attention TCP接收方的开始
     * @details optional是C++17引入的一个模板类，用于表示一个可能存在，也可能为空的值。可以看作是一个封装器，封装了一个值，这个值可能存在，也可能不存在。最常见的用途是返回一个可能失败的结果
     */
    std::optional<WrappingInt32> ackno() const;

    /**
     * @brief 应该发送给对等端的窗口大小
     * @attention 在操作上：容量减去TCPreceiver在其字节流中持有的字节数（那些已重新组装但未消耗的字节数）。
     * @attention 形式上：(a)在窗口之后的第一个字节的序列号（并且不会被接收方接受）和(b)窗口开始的序列号（确认）之间的差异。
     */
    size_t window_size() const;

    /**
     * @return 已存储但尚未重新组装的字节数
     */
    size_t unassembled_bytes() const { return _reassembler.unassembled_bytes(); }

    /**
     * @brief 处理一个入站段
     * @return 如果段的任何部分在窗口内，则返回‘ true ’
     */
    bool segment_received(const TCPSegment &seg);

    /**
     * @brief TCP接收段的输出接口
     */
    ByteStream &stream_out() { 
      return _reassembler.stream_out(); 
    }

    /**
     * @brief TCP接收段的输出接口
     */
    const ByteStream &stream_out() const {  
      return _reassembler.stream_out(); 
    }
};

#endif  // SPONGE_LIBSPONGE_TCP_RECEIVER_HH
