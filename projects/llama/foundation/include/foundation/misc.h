#pragma once
#include "pointers.h"
#include <cstddef>
#include <tuple>
#include <utility>

#if LLAMA_WIN
#define LLAMA_EXPORT __declspec(dllexport)
#define LLAMA_IMPORT __declspec(dllimport)
#else
#define LLAMA_EXPORT __attribute__((visibility("default")))
#define LLAMA_IMPORT __attribute__((visibility("default")))
#endif

#define LLAMA_CONCAT_NO_EXPAND(x, y) x##y
#define LLAMA_CONCAT(x, y) LLAMA_CONCAT_NO_EXPAND(x, y)

// 提供宏重载功能，使用方法见 LLAMA_SECOND
#define LLAMA_SELECT(prefix, n) LLAMA_CONCAT_NO_EXPAND(prefix##_, n)
#define LLAMA_GET_COUNT(_1, _2, _3, _4, _5, _6, _7, _8, _9, count, ...) count
#define LLAMA_VA_SIZE(...) LLAMA_GET_COUNT(__VA_ARGS__, 9, 8, 7, 6, 5, 4, 3, 2, 1)
#define LLAMA_VA_SELECT(prefix, ...) LLAMA_SELECT(prefix, LLAMA_VA_SIZE(__VA_ARGS__))(__VA_ARGS__)

// 不管传几个参，只留第二个
#define LLAMA_SECOND(...) LLAMA_VA_SELECT(LLAMA_SECOND, __VA_ARGS__)
#define LLAMA_SECOND_1(x)
#define LLAMA_SECOND_2(x, y) y
#define LLAMA_SECOND_3(x, y, z) y
#define LLAMA_SECOND_4(x, y, z, w) y

// 展开出一个逗号，等于多了一个参数
#define LLAMA_FUNNY1 , LLAMA_EXPORT
// LLAMA_##mod##_EXPORT 应该定义为 1 或者未定义。否则出问题。
#define LLAMA_HELPER1(mod) LLAMA_##mod##_EXPORT

/// 在导出函数前加这个前缀。 mod 为该函数属于的模块名称。
/// 如果当前正在编译的模块名称为 mod ，则展开成导出前缀。如果当前在编译的是下游模块，展开成导入前缀。
/// 在Windows上，导出/导入前缀为 `__declspec(dllexport)` / `__declspec(dllimport)`。
/// 其他平台皆为 `__attribute__((visibility ("default")))` 。
///
/// @details 实现原理：
/// ```
/// LLAMA_CONCAT(LLAMA_FUNNY, LLAMA_HELPER1(mod))
/// ```
/// 要不展开为 LLAMA_FUNNY1 ， 要不展开为 LLAMA_FUNNYLLAMA_xxx_EXPORT 。
/// LLAMA_FUNNY1 展开出一个逗号，等于凭空变出一个参数。此时第二个参数是 LLAMA_EXPORT。
/// LLAMA_FUNNYLLAMA_xxx_EXPORT 没有定义，所以还是 1 个参数（包括前面的 2 ）。此时第二个参数是 LLAMA_IMPORT。
/// 最后 LLAMA_SECOND 展开成 LLAMA_EXPORT 或者 LLAMA_IMPORT 。
#define LLAMA_API(mod) LLAMA_SECOND(2 LLAMA_CONCAT(LLAMA_FUNNY, LLAMA_HELPER1(mod)), LLAMA_IMPORT)

namespace llama
{

template <typename P> std::tuple<> FillWithSame(P &&param)
{
    return std::tuple<>{};
}

template <typename P, typename T1, typename... Ts> std::tuple<T1, Ts...> FillWithSame(P &&param)
{
    return std::tuple_cat(std::tuple<T1>{std::forward<P>(param)}, FillWithSame<P, Ts...>(std::forward<P>(param)));
}

template <typename... Interfaces> class multi : public std::tuple<p<Interfaces>...>
{
public:
    template <typename Arg>
    multi(Arg &&arg) : std::tuple<p<Interfaces>...>{FillWithSame<Arg, p<Interfaces>...>(std::forward<Arg>(arg))}
    {
    }
};

} // namespace llama