#ifndef SPONGE_LIBSPONGE_STREAM_REASSEMBLER_HH
#define SPONGE_LIBSPONGE_STREAM_REASSEMBLER_HH

#include "byte_stream.hh"

#include <cstdint>
#include <string>
#include <bits/stdc++.h>
// 一个类，它将字节流中的一系列摘录（可能是无序的，也可能是重叠的）组装成有序的字节流。
class StreamReassembler {
  private:

    // 按顺序重新组装的字节流
    ByteStream _output; 

    // 最大字节数
    size_t _capacity;   

  public:

    /**
     * @brief 构造函数
     * @param[in] capacity 流重组器的容积
     */
    StreamReassembler(const size_t capacity);

    /**
     * @brief 接收一个子字符串并将任何新的连续字节写入流。
     * @param[in] data 正在添加的字符串
     * @param[in] index data 中第一个字节的索引
     * @param[in] eof 该段是否以流的结尾结束
     * @attention 此函数从逻辑流中接受一个子字符串（也称为字节段），可能是无序的，需要组装任何新的连续子字符串并按顺序将它们写入输出流。
     */
    void push_substring(const std::string &data, const uint64_t index, const bool eof);

    /**
     * @brief 访问重新组装的字节流
     */
    const ByteStream &stream_out() const { return _output; }
    /**
     * @brief 访问重新组装的字节流
     */
    ByteStream &stream_out() { return _output; }
  
    /**
     * @brief 子字符串中已存储但尚未重新组装的字节数，即已经进入流重组器，但是还没有被写入流缓冲区的
     * @attention 如果特定索引处的字节被提交了两次，则在此函数中应该只计算一次。
     */
    size_t unassembled_bytes() const;

    /**
     * @brief 内部状态是空的（除了输出流）吗？
     * @return 如果没有子字符串等待组装，则返回true
     */
    bool empty() const;
};

#endif  // SPONGE_LIBSPONGE_STREAM_REASSEMBLER_HH
