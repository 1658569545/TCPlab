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
    :_capacity(capacity){

}

size_t ByteStream::write(const string &data) {
    size_t len=data.size();
    
    // 丢弃一部分
    if(len>_capacity - _buffer.size()){
        len=_capacity - _buffer.size();
    }
    _write_count +=len;
    for(size_t i=0;i<len;i++){
        _buffer.push_back(data[i]);
    }
    return len;
}


string ByteStream::peek_output(const size_t len) const {
    size_t length=len;
    // 丢弃一部分
    if(length >_buffer.size()){
        length = _buffer.size();
    }
    
    stringstream ss;
    for(size_t i=0;i<length;i++){
        ss<<_buffer[i];
    }
    return ss.str();
}

void ByteStream::pop_output(const size_t len) { 
    size_t length=len;
    if(length>_buffer.size()){
        length=_buffer.size();
    }
    _read_count += length;
    while(length--){
        _buffer.pop_front();
    }
    return ;
}

void ByteStream::end_input() {
    _input_ended_flag=true;
}

bool ByteStream::input_ended() const {
    return _input_ended_flag;
}

size_t ByteStream::buffer_size() const { 
    return _buffer.size();
}

bool ByteStream::buffer_empty() const {
    return _buffer.empty();
}   

bool ByteStream::eof() const {
    // 只有缓冲区为空且流已经结束
    return _buffer.empty() && input_ended();
}

size_t ByteStream::bytes_written() const {
    return _write_count;
}

size_t ByteStream::bytes_read() const {
    return _read_count;
}

size_t ByteStream::remaining_capacity() const {
    return _capacity-_buffer.size();
}
