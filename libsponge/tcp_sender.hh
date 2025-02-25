#ifndef SPONGE_LIBSPONGE_TCP_SENDER_HH
#define SPONGE_LIBSPONGE_TCP_SENDER_HH

#include "byte_stream.hh"
#include "tcp_config.hh"
#include "tcp_segment.hh"
#include "wrapping_integers.hh"

#include <functional>
#include <queue>

/**
 * @brief TCP的发送端
 * @details 接受字节流，将其分成段并发送这些段，跟踪哪些段仍在运行，维护重传计时器，并在重传计时器到期时重传正在运行的段。
 */
class TCPSender {
  private:
    /// @brief 初始序列号，在数值上为SYN端的序列号
    WrappingInt32 _isn;

    /// @brief TCPSender想要发送的段的出站队列
    std::queue<TCPSegment> _segments_out{};

    /// @brief 连接的重传计时器，即重传超时（RTO）的初始值
    unsigned int _initial_retransmission_timeout;

    /// @brief 尚未发送的传出字节流
    ByteStream _stream;

    /// @brief 要发送的下一个字节的（绝对）序列号
    uint64_t _next_seqno{0};

    /// @brief 已经发送但是没有确认的TCPSegment
    std::queue<TCPSegment> segments_unconfirmed ;

    /// @brief 记录已发送但未被确认的字节总数
    size_t _bytes_in_flight=0;

    /// @brief 记录连续重传的次数
    size_t _consecutive_retransmissions=0;

    /// @brief 接收方已确认的最高绝对序列号（即下一个期望接收的字节的绝对序列号）
    size_t _recv_ackno = 0;

    /// @brief 自上次超时事件以来经过的时间（毫秒）
    size_t _timer=0;

    /// @brief RTO
    size_t _retransmission_timeout=0;

    /// @brief 指示重传定时器是否正在运行
    bool _timer_running=false;

    /// @brief SYN标志，标记是否已经发送了SYN报文段
    bool syn_flag=false;

    /// @brief FIN标志，标记是否已经发送了FIN报文段
    bool fin_flag=false;
  public:
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
     * @param[in] ackno 远端接收方的确认号
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
     */
    void fill_window();

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
     * @brief TCPSender已排队等待传输的报文段。
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

    
};

#endif  // SPONGE_LIBSPONGE_TCP_SENDER_HH
