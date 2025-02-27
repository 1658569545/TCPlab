#include "tcp_receiver.hh"

// 窗口的左边界为ackno，ackno是接收方希望找到的第一个字节的序列号
// 窗口的右边界，是接收方不再愿意接受的第一个字节的索引。
// 窗口大小 = 接收端的容量 - 接收端缓冲区存储的字节数
// 接收端缓冲区即为BytesStream，存储已经重组好的数据段

// 整个接收端由两部分组成，缓冲区和窗口，然后在窗口中使用stream_reassembler进行数据段重组

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

bool TCPReceiver::segment_received(const TCPSegment &seg) {
    // 记录TCP数据段的有效载荷长度
    size_t length=0;
    // 记录当前TCP数据段有效载荷的首个绝对序列号
    size_t abs_seqno=0;
    // 记录是否接收了SYN/FIN段
    bool ret=false;

    // 判断SYN段
    if(seg.header().syn){
        // 重复的SYN，直接丢弃
        if(_SYN_flag){
            return false;
        }
        ret=true;
        _SYN_flag=true;
        // 设置_isn
        _isn=seg.header().seqno.raw_value();
        // _base右移，SYN会消耗一个序列号
        _base=1;

        // 设置绝对序列号
        abs_seqno=1;

        // 获取有效载荷,SYN不是有效载荷，但是会消耗一个序列
        size_t temp = seg.length_in_sequence_space()-1;
        if(temp==0){
            return true;
        }
    }
    // 不是SYN，但之前没有收到过SYN段，因此直接丢弃
    else if(!_SYN_flag){
        return false;
    }
    // 正常数据段
    else{
        // 计算绝对序列号
        abs_seqno=unwrap(
        WrappingInt32(seg.header().seqno.raw_value())
        ,WrappingInt32(_isn)
        ,_base);
    }

    // 获取载荷长度,包含SYN/FIN
    length=seg.length_in_sequence_space();

    // 判断是否是FIN段
    if(seg.header().fin){
        if(_FIN_flag){
            return false;
        }
        _FIN_flag=true;
        ret=true;
    }

    // 判断是否在窗口外
    if(_base + window_size() <= abs_seqno ||abs_seqno + length <=_base  ){
        // 如果不是SYN/FIN段，即是正常数据段
        if (!ret) {
            return false;
        }
    }


    // 开始重组数据，注意abs_seqno是TCP绝对序列号，会计算SYN，而此时我们需要的索引是针对流的，而流忽略了SYN，因此需要-1.
    _reassembler.push_substring(seg.payload().copy(),abs_seqno-1,seg.header().fin);
    
    // 窗口右移
    // 但是窗口的绝对序列不会忽略SYN，而流重组器会忽略SYN，因此为head_index+1，
    _base=_reassembler.get_Head_index()+1;

    // 流结束了
    if(_reassembler.input_ended()){
        // FIN会消耗一个序列
        _base+=1;
    }
    

    return true;
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    // _base是接收端窗口期望的下一个字节的绝对序列号，即接收窗口中第一个未被确认的字节位置。
    // 所以当最开始的时候，缓冲区接收的数据为0,因此接收端窗口会顶在最前面，
    // 而由于SYN会占一个序列号，因此如果接收到了有SYN的数据段，那么_base必定大于0。
    // 因此也会对ISN进行设置
    if(_base>0){
        // _base是绝对序列号
        return WrappingInt32(wrap(_base,WrappingInt32(_isn)));
    }
    else{
        return std::nullopt;
    }
}

size_t TCPReceiver::window_size() const {
    return _capacity-_reassembler.stream_out().buffer_size();
}
