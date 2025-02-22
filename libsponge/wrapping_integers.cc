#include "wrapping_integers.hh"

// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;


WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {
    // 将绝对值转换为相对值
    // 相对值=绝对值+初始序列号（可以联想操作系统中内存管理的地址转换）
    return static_cast<WrappingInt32>(n)+isn.raw_value();
}

uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    // 给定一个相对序列号n、初始序列号（isn）、以及一个绝对的检查点序列号，计算出与n最接近检查点的绝对序列号
    // 需要使用检查点，因为一个序列号对应多个绝对序列号。
    // 为什么说一个序列号可能对应多个绝对序列号，

    // 具体来说，假设TCP的初始序列号（ISN）是一个随机值，并且数据流可能很长。当发送的数据量超过了32位序列号的范围时，
    // 序列号就会回绕。例如：第一个发送的数据字节可能会有序列号17。如果数据很长，如果ISN为232,那么再次出现的17可能是232 + 17 + 2^32
    // 因此就需要检查点checkpoint来判断对应哪个绝对序列号
    // checkpoint一般设置为最近解包的绝对序列号
    
    // 存储结果
    uint64_t res;
    
    // 先计算偏移量
    uint32_t offset=n.raw_value()-isn.raw_value();
    
    // 通过将checkpoint的高32位与offset相加来计算初步的绝对序列号 t（只是候选值，因为可能会对应多个绝对序号）
    // 与高32位相加是因为要转换为64位
    uint64_t t=(checkpoint & 0xFFFFFFFF00000000) + offset;

    res=t;

    // 确定与检查点最接近的绝对序列号：

    // 如果t+2^32与checkpoint的差值绝对值小于t与checkpoint的差值，那么将t加上2^32，
    // 即认为序列号回绕过一次，选择一个更大的绝对序列号。1ul<<32即为2^32
    if(abs(int64_t(t+(1ul<<32)-checkpoint))<abs(int64_t(t-checkpoint))){
        res=t+(1ul<<32);
    }
    
    // 同理检查可能的下溢
    if(t>=(1ul<<32) && abs(int64_t(t-(1ul<<32)-checkpoint))<abs(int64_t(t-checkpoint))){
        res = t - (1ul << 32);
    }

    // 该部分总结，只需要检查好环绕一次、下溢、数据转换即可

    return res;
}
