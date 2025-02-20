#include "stream_reassembler.hh"

//流重组器的虚拟实现。

//对于Lab 1，请替换为通过‘ make check_lab1 ’运行的自动检查的真实实现。

//你需要在‘ stream_reassembly .hh ’中的类声明中添加私有成员

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _output(capacity), _capacity(capacity),reabuffer() {

}

long StreamReassembler::splice_node(node &eml1,const node &eml2){
    node x,y;
    // x代表放在前面
    if(eml1.begin>eml2.begin){
        x=eml2;
        y=eml1;
    }
    else{
        x=eml1;
        y=eml2;
    }

    // 没有交叉
    if(x.begin+x.length<y.begin){
        return -1;
    }
    // 完全覆盖
    else if(x.begin+x.length>=y.begin+y.length){
        eml1=x;
        return y.length;
    }
    // 部分重叠
    else{
        string str=y.data.substr(x.begin+x.length-y.begin);
        eml1.data=x.data+str;
        eml1.begin=x.begin;
        eml1.length=eml1.data.size();
        return x.begin+x.length-y.begin;
    }
}

void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    
    // 整体思路就类似于操作系统的空闲内存管理，需要判断是否和前后的空闲内存碎片进行合并

    // 如果首索引+容积 <= data的第一个字节索引index，则代表无法再放入了，因为没有容积了
    if(_head_index + _capacity <= index){
        return ;
    }

    node elm;
    
    // 整个数据段完全落在了前面
    if(index+data.size()<=_head_index){
        if(eof){
            eof_flag=true;
        }
        if(eof_flag && empty()){
            _output.end_input();
        }
        return ;
    }
    // data有部分数据超过了重组器的前界限
    else if(index<_head_index){
        // 计算偏移，也就是data要丢弃的部分,
        size_t offset=_head_index-index;
        // 只保留落在缓冲区的部分
        elm.data.assign(data.begin()+offset,data.end());
        
        elm.begin=_head_index;
    }
    // 整个数据段完全落在了后面
    else if(index>=_head_index + _capacity){
        return ;
    }
    // data有部分数据超过了重组器的后界限
    else if(index < (_head_index+_capacity) && (index+data.size())>(_head_index+_capacity)){
        elm.begin=index;
        elm.data=data.substr(0,(_head_index+_capacity-index));
    }
    // 完全落在重组器中间
    else {
        elm.begin=index;
        elm.data=data;
    }

    elm.length=elm.data.size();
    
    // 未处理数目增加
    _unassembled_byte+=elm.length;

    // 获取插入位置
    auto iter=reabuffer.lower_bound(elm);

    // 判断能否和后面进行合并
    while(iter!=reabuffer.end()){
        long merge_bytes =splice_node(elm,*iter);
        if(merge_bytes<0){
            break;
        }
        reabuffer.erase(iter);
        _unassembled_byte-=merge_bytes;
        // 去除之后需要重新寻找
        iter = reabuffer.lower_bound(elm);
    }
    
    /// 重新获取插入位置
    iter = reabuffer.lower_bound(elm);

    /// 判断能否和前面合并
    while (iter != reabuffer.begin()) {
        auto prev = std::prev(iter);
        long merge_bytes = splice_node(elm, *prev);
        if (merge_bytes < 0) {
            break;
        }
        _unassembled_byte -= merge_bytes;
        reabuffer.erase(prev);
        iter = reabuffer.lower_bound(elm);
    }

    reabuffer.insert(elm);
    
    // 很明显，如果缓冲区中第一个数据段的首索引不等于重组器的首索引，那么代表该数据段前面还有数据没有到达
    if(!reabuffer.empty() && reabuffer.begin()->begin==_head_index){
        // 如果满足上述条件，第一个就肯定可以
        const node front=*reabuffer.begin();
        size_t written=_output.write(front.data);
        // 容器的首索引需要往后面移动
        _head_index+=written;
        _unassembled_byte-=written;
        reabuffer.erase(reabuffer.begin());

    }
    if(eof){
        eof_flag=true;
    }
    if (eof_flag && empty()) {
        _output.end_input();
    }

    
}

size_t StreamReassembler::unassembled_bytes() const {
    return _unassembled_byte;
}

bool StreamReassembler::empty() const {
    return _unassembled_byte==0;
}
