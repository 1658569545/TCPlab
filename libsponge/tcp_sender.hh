#ifndef SPONGE_LIBSPONGE_TCP_SENDER_HH
#define SPONGE_LIBSPONGE_TCP_SENDER_HH

#include "byte_stream.hh"
#include "tcp_config.hh"
#include "tcp_segment.hh"
#include "wrapping_integers.hh"

#include <functional>
#include <queue>
#include<sstream>
/**
 * @brief TCP的发送端
 * @details 接受字节流，将其分成段并发送这些段，跟踪哪些段仍在运行，维护重传计时器，并在重传计时器到期时重传正在运行的段。
 */
class TCPSender {
  private:
    /// @brief 初始序列号
    WrappingInt32 _isn; 

    /// @brief 发送队列
    std::queue<TCPSegment> _segments_out{};

    /// @brief 初始的重传时间
    unsigned int _initial_retransmission_timeout;

    /// @brief 输出流
    ByteStream _stream;

    /// @brief 要发送的下一个绝对序列号
    uint64_t _next_seqno{0};

    /// @brief 已经接收的最大确认序列号（绝对）
    uint64_t recv_ackno=0;

    /// @brief SYN标志
    bool syn_flag=false;

    /// @brief fin标志
    bool fin_flag=false;

    /// @brief 初始RTO
    size_t rto=0;

    /// @brief 重传次数
    size_t retransmissions=0;

    /// @brief 已经发送但是没有收到确认的TCP段队列
    std::queue<TCPSegment> _segments_outstanding{};

    /// @brief 已经发送但是没有确认的字节数
    size_t bytes=0;

    /// @brief 超时重传计时器是否正在启动
    bool timer_running=false;

    /// @brief 从超时重传计时器启动之后到现在的时间
    size_t timer=0;

    /// @brief 窗口大小
    size_t _window_size=0;
  public:
    
    /**
     * @brief 封装一个发送段，用来复用
     * @param[in] seg 要发送的段
     */
    void send_segment(TCPSegment &seg);
    
    /**
     * @brief 构造函数
     * @param[in] capacity 输出字节流的容量
     * @param[in] retx_timeout 在重传最老的未完成段之前等待的初始时间
     * @param[in] fixed_isn 如果设置了该变量的值，则作为_isn使用，否则使用随机值来作为isn
     */
    TCPSender(const size_t capacity = TCPConfig::DEFAULT_CAPACITY,
              const uint16_t retx_timeout = TCPConfig::TIMEOUT_DFLT,
              const std::optional<WrappingInt32> fixed_isn = {});

    /**
     * @brief 获取尚未发送的传出字节流
     */
    ByteStream &stream_in() { 
      return _stream; 
    }

    /**
     * @brief 获取尚未发送的传出字节流
     */
    const ByteStream &stream_in() const { 
      return _stream; 
    }
 
    /**
     * @brief 是否收到了新的确认
     * @param[in] ackno 远端接收方的确认号,即期待发送方的下一个序号
     * @param[in] window_size 远程接收器的窗口大小
     * @details 可以导致TCPSender发送一个段的方法
     * @attention 采用累积确认
     * @return 如果确认无效（确认TCPSender尚未发送的内容），返回‘ false ’
     */
    bool ack_received(const WrappingInt32 ackno, const uint16_t window_size);

    /**
     * @brief 生成空有效载荷段（用于创建空ACK段）
     * @attention 无参版本
     */
    void send_empty_segment();

    /**
     * @brief 创建和发送段来填充尽可能多的窗口
     * @attention 参数是供lab4使用的，表示是否发送SYN段，对于lab3无影响，本来想在lab4中操作syn_flag，但是该变量是私有的，就没有考虑
     */
    void fill_window(bool send_syn=true);

    /**
     * @brief 通知TCPSender时间的流逝
     * @param[in] ms_since_last_tick 自上次调用此方法以来的毫秒数
     */
    void tick(const size_t ms_since_last_tick);

    /**
     * @brief 计算有多少序列号被发送但尚未确认
     * @attention SYN和FIN各计数一个字节
     * @details 详见TCPSegment: length_in_sequence_space ()
     */
    size_t bytes_in_flight() const;

    /**
     * @brief 返回连续重传的次数
     */
    unsigned int consecutive_retransmissions() const;

    /**
     * @brief 获取TCPSender已排队等待传输的报文段。
     * @attention 这些数据必须从队列中取出并由TCPConnection发送，TCPConnection需要在发送之前填写由TCPReceiver设置的字段（确认号和窗口大小）。
     */
    std::queue<TCPSegment> &segments_out() { 
      return _segments_out; 
    }

    /**
     * @brief 获取要发送的下一个字节的绝对序列号
     */
    uint64_t next_seqno_absolute() const { 
      return _next_seqno; 
    }

    /**
     * @brief 获取要发送的下一个字节的相对顺序
     */
    WrappingInt32 next_seqno() const { 
      return wrap(_next_seqno, _isn); 
    }
    
    /**
     * @brief 获取syn_flag标志
     * @attention lab4使用
     */
    bool _syn_send() const{
      return syn_flag;
    }

    /**
     * @brief 获取fin_flag标志
     * @attention lab4使用
     */
    bool _fin_send() const{
      return fin_flag;
    }
};

#endif  // SPONGE_LIBSPONGE_TCP_SENDER_HH
