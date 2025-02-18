#ifndef SPONGE_LIBSPONGE_BYTE_STREAM_HH
#define SPONGE_LIBSPONGE_BYTE_STREAM_HH

#include <cstddef>
#include <cstdint>
#include <deque>
#include <list>
#include <string>
#include <utility>

// 实现一个可靠的字符串流

class ByteStream {
  private:
    // 缓冲区容积
    size_t _capacity=0;
    // 已经读了的数据数目
    size_t _read_count=0;
    // 已经写了的数据数目
    size_t _write_count=0;
    // 缓冲区
    std::deque<char>_buffer={};
    // 是否已经到达末尾
    bool _input_ended_flag = false;
    // 指示流发生错误的标志。
    bool _error=false; 

  public:
    /**
     * @breif 构造函数
     * @param[in] capacity 缓冲区容积
     */
    ByteStream(const size_t capacity);

    /**
     * @brief 读取数据
     * @param[in] data 数据
     * @return 返回成功读取数据的长度
     */
    size_t write(const std::string &data);

    /**
     * @return 流有空间容纳的额外字节数
     */
    size_t remaining_capacity() const;

    /**
     * @brief 字节流到达其结束的信号
     */
    void end_input();

    /**
     * @brief 设置错误标志
     */
    void set_error() { _error = true; }

    /**
     * @brief 从缓冲区的输出端复制字节
     * @param[in] len 复制的字符串长度
     * @return 返回复制的字符串
     */
    std::string peek_output(const size_t len) const;

    /**
     * @brief 从缓冲区的输出端进行删除
     * @param[in] len 删除的长度
     */
    void pop_output(const size_t len);

    /**
     * @brief 读取数据
     * @param[in] len 要读取的长度
     * @return 返回读取的字符串
     */
    std::string read(const size_t len) {
        const auto ret = peek_output(len);
        pop_output(len);
        return ret;
    }

    /**
     * @return 如果流已经结束，返回true
     */
    bool input_ended() const;

    /**
     * @return 如果流发生错误，返回true
     */
    bool error() const { return _error; }

    /**
     * @return 当前可以从流中读取的最大数量
     */
    size_t buffer_size() const;

    /**
     * @return 如果缓冲区为空，则为true
     */
    bool buffer_empty() const;

    /**
     * @return 如果输出到达结尾，则为true
     */
    bool eof() const;

    /**
     * @return 写入的总字节数
     */
    size_t bytes_written() const;

    /**
     * @return 读取的总字节数
     */
    size_t bytes_read() const;
};

#endif  // SPONGE_LIBSPONGE_BYTE_STREAM_HH
