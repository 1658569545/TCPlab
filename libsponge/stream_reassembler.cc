#include "stream_reassembler.hh"

//流重组器的虚拟实现。

//对于Lab 1，请替换为通过‘ make check_lab1 ’运行的自动检查的真实实现。

//你需要在‘ stream_reassembly .hh ’中的类声明中添加私有成员

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _output(capacity), _capacity(capacity) {

}

size_t StreamReassembler::splice_node(node &eml1,const node &eml2){

}

void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    // 整体思路就类似于操作系统的空闲内存管理，需要判断是否和前后的空闲内存碎片进行合并

    // 如果首索引+容积 < data的第一个字节索引index，则代表无法再放入了，因为没有容积了
    if(_head_index + _capacity < index){
        return ;
    }
    node elm;

}

size_t StreamReassembler::unassembled_bytes() const {
    return _unassembled_byte;
}

bool StreamReassembler::empty() const {
    return _unassembled_byte==0;
}
