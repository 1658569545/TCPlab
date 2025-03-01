#ifndef SPONGE_LIBSPONGE_STREAM_REASSEMBLER_HH
#define SPONGE_LIBSPONGE_STREAM_REASSEMBLER_HH

#include "byte_stream.hh"

#include <cstdint>
#include <string>
#include <bits/stdc++.h>
#include <set>
#include <vector>
// 一个类，它将字节流中的一系列摘录（可能是无序的，也可能是重叠的）组装成有序的字节流。
class StreamReassembler {
  private:
    // 
    /**
     * @brief 用来描述各个到来的无序字节段
     * @param[in] begin 字节段的首索引
     * @param[in] length 字节段长度
     * @param[in] data 内容
     */
    struct node{
      size_t begin=0;
      size_t length=0;
      std::string data="";
      
      /**
       * @brief 重写比较规则
       * @details this<t
       */
      bool operator<(const node t)const{
        return begin<t.begin;
      }
    };

    /// @brief 未装配的字节数
    size_t _unassembled_byte = 0;

    /// @brief 存放被流重组器按顺序重新组装的字节流
    ByteStream _output; 

    /// @brief 最大字节数
    size_t _capacity;  
    
    /// @brief eof标记
    bool eof_flag=false;

    /// @brief 存储数据段 
    std::set<node>reabuffer;

    /// @brief 重组器中第一个字节索引，所有小于该索引的字节均已写入字节流中。也就是下一个需要处理的字节的位置
    size_t _head_index=0;

    /**
     * @brief 对两个数据段进行拼接
     * @param[in] eml1 第一个数据段
     * @param[in] eml2 第二个数据段
     * @return 返回两个数据段重叠的字节数，用来在后续判断是否合并
     */
    long splice_node(node &eml1,const node &eml2);

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
     * @brief 访问ByteStream
     */
    ByteStream &stream_out() { return _output; }
  
    /**
     * @brief 数据段中已存储但尚未重新组装的字节数，即已经进入流重组器，但是还没有被写入流缓冲区的
     * @attention 如果特定索引处的字节被提交了两次，则在此函数中应该只计算一次。
     */
    size_t unassembled_bytes() const;

    /**
     * @brief 内部状态是空的（除了输出流）吗？
     * @return 如果没有子字符串等待组装，则返回true
     */
    bool empty() const;

    /**
     * @brief 获取重组器的首索引,即下一个需要处理的字节的索引
     */
    size_t get_Head_index() const {
      return _head_index;
    }

    /**
     * @brief 判断缓冲区中的字节流是否已经结束
     */
    bool input_ended() const {
      return _output.input_ended();
    }
};

#endif  // SPONGE_LIBSPONGE_STREAM_REASSEMBLER_HH
