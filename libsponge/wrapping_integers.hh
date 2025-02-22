#ifndef SPONGE_LIBSPONGE_WRAPPING_INTEGERS_HH
#define SPONGE_LIBSPONGE_WRAPPING_INTEGERS_HH

#include <cstdint>
#include <ostream>

/**
 * @brief WrappingInt32 用来表示序列号（start at ISN）
 * @details 由于TCP序列号的首个序列号，一般不从0开始（加强安全性），因此需要从一个随机值ISN开始，因此ISN又称为初始序列号1
 */
class WrappingInt32 {
  private:
    /// @brief 原始32位存储整数，存储初始序列号ISN
    uint32_t _raw_value;  

  public:
    
    /**
     * @brief 构造函数
     * @attention explicit禁止隐式转换
     */
    explicit WrappingInt32(uint32_t raw_value) : _raw_value(raw_value) {

    }

    /**
     * @brief 获取初始序列号
     */
    uint32_t raw_value() const { 
      return _raw_value; 
    }  
};

/**
 * @brief 将64位绝对序列号（start at 0）转换为32位相对序列号(start at ISN)
 * @param[in] n 绝对序列号
 * @param[in] isn 初始序列号
 * @return 返回一个32位的相对序列号
 */
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn);

/**
 * @brief 将32位相对序列号（ISN）转换为64位绝对序列号（start at 0）
 * @param[in] n 相对序列号
 * @param[in] isn 初始序列号
 * @param[in] checkpoin 最近的64位绝对序列号
 * @return 返回包装到“n”并最接近“checkpoint”的绝对序列号。
 * @attention TCP连接的两个流中的每一个都有自己的ISN。一个流从本地TCPSender运行到远程TCPReceiver，具有一个ISN，另一个流从远程TCPSender运行到本地TCPReceiver，具有不同的ISN。
 */
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint);

/**
 * @brief 辅助函数 减
 */
inline int32_t operator-(WrappingInt32 a, WrappingInt32 b) { 
  return a.raw_value() - b.raw_value(); 
}

/**
 * @brief 辅助函数 ==
 */
inline bool operator==(WrappingInt32 a, WrappingInt32 b) { 
  return a.raw_value() == b.raw_value(); 
}

/**
 * @brief 辅助函数 ！=
 */
inline bool operator!=(WrappingInt32 a, WrappingInt32 b) { 
  return !(a == b); 
}

/**
 * @brief 将a输入到流os中
 */
inline std::ostream &operator<<(std::ostream &os, WrappingInt32 a) { 
  return os << a.raw_value(); 
}

//! \brief The point `b` steps past `a`.
inline WrappingInt32 operator+(WrappingInt32 a, uint32_t b) { 
  return WrappingInt32{a.raw_value() + b}; 
}

//! \brief The point `b` steps before `a`.
inline WrappingInt32 operator-(WrappingInt32 a, uint32_t b) { 
  return a + -b; 
}


#endif  // SPONGE_LIBSPONGE_WRAPPING_INTEGERS_HH
