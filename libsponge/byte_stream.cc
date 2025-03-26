#include "byte_stream.hh"

#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <sstream>
// 实现一个可靠的字符串流

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity) 
    :_capacity(capacity)
    {

}

//***************************deque 

size_t ByteStream::write(const string &data) {
    size_t len=data.size();
    
    // 丢弃一部分
    if(len>_capacity - _buffer.size()){
        len=_capacity - _buffer.size();
    }
    _write_count +=len;
    // 批量插入，代替push_back
    _buffer.insert(_buffer.end(),data.begin(),data.begin()+len);
    return len;
}

string ByteStream::peek_output(const size_t len) const {
    const size_t copy_len = std::min(len, _buffer.size());
    // 返回临时对象，借用移动构造函数
    return string(_buffer.begin(), _buffer.begin() + copy_len);
}

void ByteStream::pop_output(const size_t len) { 
    size_t length=len;
    if(length>_buffer.size()){
        length=_buffer.size();
    }
    _read_count += length;
    // 批量删除
    _buffer.erase(_buffer.begin(),_buffer.begin()+length);
    return ;
}


//***************************循环队列 
/*
size_t ByteStream::write(const string &data){
    // 判断
    if (_input_ended_flag || data.empty()){
        return 0;
    }
    // 计算需要写入的字符长度
    size_t writelen=min(data.size(),remaining_capacity());
    if(writelen==0){
        return 0;
    }

    // 计算队尾位置
    const size_t rear = (_front + _size) % _capacity; 

    // 分段写入，可能需要两次，第一次是从队尾写到缓冲区尾，然后第二段则是从缓冲区头部开始写。因为是循环队列 
    // 第一次写入，计算从队尾到缓冲区尾还有多少空间
    const size_t first_chunk = std::min(writelen,_capacity-rear);
    std::move(data.begin(),data.begin()+first_chunk,_buffer.begin()+rear);

    // 第二次写入，如果要写的字节总数>第一次写的总数，则需要继续从缓冲区开始写
    if(writelen>first_chunk){
        std::move(data.begin()+first_chunk, data.begin() + writelen, _buffer.begin());
    }
    _size+=writelen;
    _write_count+=writelen;
    return writelen;
}

std::string ByteStream::peek_output(const size_t len) const {
    const size_t peek_len = std::min(len, _size);
    if (peek_len == 0) return "";

    const size_t first_chunk = std::min(peek_len, _capacity - _front);
    
    std::string result;
    result.reserve(peek_len);  // 预分配空间提升性能

    // 第一部分：从_front到缓冲区末尾
    result.append(_buffer.begin() + _front, 
                  _buffer.begin() + _front + first_chunk);

    // 第二部分：从缓冲区头部继续读取剩余数据
    if (peek_len > first_chunk) {
        result.append(_buffer.begin(), 
                       _buffer.begin() + (peek_len - first_chunk));
    }

    return result;
}

void ByteStream::pop_output(const size_t len) {
    size_t poplen=min(len,_size);
    // 移动队头指针
    _front=(_front+poplen)%_capacity;
    _size-=poplen;
    _read_count+=poplen;
}
*/

void ByteStream::end_input() {
    _input_ended_flag=true;
}

bool ByteStream::input_ended() const {
    return _input_ended_flag;
}

size_t ByteStream::buffer_size() const { 
    // return _size; 循环队列
    return _buffer.size();  
}

bool ByteStream::buffer_empty() const {
    // return _size==0; 循环队列
    return buffer_size()==0;
}   

bool ByteStream::eof() const {
    // 只有缓冲区为空且流已经结束
    return buffer_empty() && input_ended();
}

size_t ByteStream::bytes_written() const {
    return _write_count;
}

size_t ByteStream::bytes_read() const {
    return _read_count;
}

size_t ByteStream::remaining_capacity() const {
    // return _capacity - _size; 循环队列
    return _capacity-_buffer.size();
}
