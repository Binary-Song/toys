#pragma once
#include <type_traits>

/// 为 scoped enum 类型 `T` 开启位运算支持。需在 llama 命名空间使用
#define LLAMA_ENABLE_BITWISE_OPS(T)                                                                                    \
    template <> struct AllowBitwiseOps<T>                                                                              \
    {                                                                                                                  \
        static constexpr bool value = true;                                                                            \
    }

namespace llama
{

template <typename T> struct AllowBitwiseOps
{
    static constexpr bool value = false;
};

/***********************************************************************************

 位运算符 for 枚举

**************************************************************************************/

template <typename T, typename = std::enable_if_t<std::is_enum_v<T> && llama::AllowBitwiseOps<T>::value>>
inline T operator~(const T &value)
{
    return static_cast<T>(~static_cast<std::underlying_type_t<T>>(value));
}

template <typename T, typename = std::enable_if_t<std::is_enum_v<T> && llama::AllowBitwiseOps<T>::value>>
inline T operator&(const T &left, const T &right)
{
    return static_cast<T>(static_cast<std::underlying_type_t<T>>(left) & static_cast<std::underlying_type_t<T>>(right));
}

template <typename T, typename = std::enable_if_t<std::is_enum_v<T> && llama::AllowBitwiseOps<T>::value>>
inline T operator|(const T &left, const T &right)
{
    return static_cast<T>(static_cast<std::underlying_type_t<T>>(left) | static_cast<std::underlying_type_t<T>>(right));
}

template <typename T, typename = std::enable_if_t<std::is_enum_v<T> && llama::AllowBitwiseOps<T>::value>>
inline T operator^(const T &left, const T &right)
{
    return static_cast<T>(static_cast<std::underlying_type_t<T>>(left) ^ static_cast<std::underlying_type_t<T>>(right));
}

template <typename T, typename = std::enable_if_t<std::is_enum_v<T> && llama::AllowBitwiseOps<T>::value>>
inline T &operator&=(T &left, const T &right)
{
    return left = (left & right);
}

template <typename T, typename = std::enable_if_t<std::is_enum_v<T> && llama::AllowBitwiseOps<T>::value>>
inline T &operator|=(T &left, const T &right)
{
    return left = (left | right);
}

template <typename T, typename = std::enable_if_t<std::is_enum_v<T> && llama::AllowBitwiseOps<T>::value>>
inline T &operator^=(T &left, const T &right)
{
    return left = (left ^ right);
}

template <typename T, typename = std::enable_if_t<std::is_enum_v<T> && llama::AllowBitwiseOps<T>::value>>
inline bool operator+(const T &left)
{
    return static_cast<std::underlying_type_t<T>>(left);
}

template <typename T, typename = std::enable_if_t<std::is_enum_v<T> && llama::AllowBitwiseOps<T>::value>>
inline bool operator!(const T &left)
{
    return !static_cast<std::underlying_type_t<T>>(left);
}

} // namespace llama