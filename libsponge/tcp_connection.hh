#ifndef SPONGE_LIBSPONGE_TCP_FACTORED_HH
#define SPONGE_LIBSPONGE_TCP_FACTORED_HH

#include "tcp_config.hh"
#include "tcp_receiver.hh"
#include "tcp_sender.hh"
#include "tcp_state.hh"

class TCPConnection {
  private:
    TCPConfig _cfg;
    TCPReceiver _receiver{_cfg.recv_capacity};
    TCPSender _sender{_cfg.send_capacity, _cfg.rt_timeout, _cfg.fixed_isn};

    /// @brief 输出队列
    std::queue<TCPSegment> _segments_out{};

    //! Should the TCPConnection stay active (and keep ACKing)
    //! for 10 * _cfg.rt_timeout milliseconds after both streams have ended,
    //! in case the remote TCPConnection doesn't know we've received its whole stream?
    bool _linger_after_streams_finish{true};

    /// @brief 存储从收到最后一个报文段到现在的毫秒数
    size_t last_since_ack_time{0};

    bool _active = true;
    bool _need_send_rst = false;

    /**
     * @brief 发送数据
     * @param [in] send_syn 是否需要发送syn段
     * @attention 将_sender的发送队列中的数据传输到TCPConnection的发送队列中
     */
    bool push_segments_out(bool send_syn = false);

    void unclean_shutdown(bool send_rst);
    bool clean_shutdown();
    bool in_syn_recv();
    bool in_syn_sent();

  public:
    
    /**
     * @brief 发起连接
     */
    void connect();

    /**
     * @brief 将数据写入出站字节流，如果可能的话，通过TCP发送
     * @return 返回实际写入的‘data'中的字节数。
     */
    size_t write(const std::string &data);

    /**
     * @brief 获取现在可以写入的字节数。感觉是获取输出缓冲区的空闲大小
     */
    size_t remaining_outbound_capacity() const;

    /**
     * @brief 关闭出站字节流，但是不会关闭入站字节流
     */
    void end_input_stream();

    /**
     * @brief 获取从对等体接收到的入站字节流，即从对方过来的字节流
     */
    ByteStream &inbound_stream() { 
      return _receiver.stream_out(); 
    }

    /**
     * @brief 获取发送但未确认的字节数，将SYN/FIN每个计算为一个字节
     */
    size_t bytes_in_flight() const;
    
    /**
     * @brief 获取尚未重新组装的字节数
     */
    size_t unassembled_bytes() const;
    
    /**
     * @brief 获取从收到最后一个报文段到现在的毫秒数
     */
    size_t time_since_last_segment_received() const;
    
    /**
     * @brief 获取TCPConnection的当前状态，包括发送方、接收方、连接的状态
     */
    TCPState state() const { 
      return {_sender, _receiver, active(), _linger_after_streams_finish}; 
    };

    /**
     * @brief 接收TCPSegment
     * @param[in] seg 要接收的TCPSegment
     */
    void segment_received(const TCPSegment &seg);

    /**
     * @brief 周期性调用
     * @param[in] ms_since_last_tick 自上次调用该函数以来运行了多久
     * @attention 同时也能判断对方是否还存在
     */
    void tick(const size_t ms_since_last_tick);

    /**
     * @brief 获取TCP连接中已经排队等待传输的TCPSegment
     * @attention 即对于TCP来说已经发送的TCPSegment
     */
    std::queue<TCPSegment> &segments_out() { 
      return _segments_out; 
    }

    /**
     * @brief 判断连接是否仍然有效，有效返回true，无效返回false
     * @attention 如果任一流仍在运行，或者在两个流都已完成之后 TCPConnection 仍在等待（例如，等待对端的重传确认）。则返回true
     */
    bool active() const;

    /**
     * @brief 构造函数
     * @attention 从配置构造一个新连接，但是禁止隐式转换
     */
    explicit TCPConnection(const TCPConfig &cfg) : _cfg{cfg} {}

    /**
     * @brief 析构函数
     * @attention 如果连接仍然打开，析构函数发送一个RST
     */
    ~TCPConnection();  
    
    // 删除掉无参构造函数
    TCPConnection() = delete;

    /**
     * @brief 使用默认的移动构造函数
     * @details 通过“窃取”资源来避免不必要的复制,而不是创建一个副本，即允许move
     */
    TCPConnection(TCPConnection &&other) = default;
    
    /**
     * @brief 使用默认的移动赋值函数
     * @details 通过“窃取”资源来避免不必要的复制,而不是创建一个副本
     */
    TCPConnection &operator=(TCPConnection &&other) = default;

    // 禁止拷贝构造和拷贝赋值函数
    TCPConnection(const TCPConnection &other) = delete;
    TCPConnection &operator=(const TCPConnection &other) = delete;

    // 小笔记：
    // 移动构造函数和拷贝构造函数的区别
    // 拷贝构造函数：
    //    拷贝构造函数用于创建一个对象的副本（通常是通过传值或拷贝已有对象时调用），例如使用一个对象来初始化另外一个对象，string str1=str，使用str来初始化str1,
    //    能够保证两个对象的资源（如数据）是独立的，并且修改其中一个对象时不会影响另一个对象。但是可能复制会很贵
    // 移动构造函数：
    //    移动构造函数用于将一个右值对象（临时对象）转移给另一个对象，而不是创建副本。例如string str1=std::move(str)
    //    移动构造函数会“窃取”右值对象的资源（如动态内存、文件句柄等），并将其所有权转移到新对象中。被移动的对象通常会被置为一个有效的空状态，避免重复释放资源。
    //    移动比拷贝高效，因为它不需要分配新的内存或复制数据。它只是转移资源的所有权，避免了不必要的内存操作。
    //    移动操作的对象（右值对象）会进入一个有效但未定义的状态，因此不能继续使用原对象中的资源
};

#endif  // SPONGE_LIBSPONGE_TCP_FACTORED_HH
