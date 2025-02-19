#include "stream_reassembler.hh"

//流重组器的虚拟实现。

//对于Lab 1，请替换为通过‘ make check_lab1 ’运行的自动检查的真实实现。

//你需要在‘ stream_reassembly .hh ’中的类声明中添加私有成员

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _output(capacity), _capacity(capacity) {}


void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    
}

size_t StreamReassembler::unassembled_bytes() const {

}

bool StreamReassembler::empty() const {

}
