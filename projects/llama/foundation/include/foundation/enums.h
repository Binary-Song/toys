/// @file
/// llama的所有枚举类型都定义在这里。以便拓展。
///
/// 如果枚举类型支持位运算（各枚举项非互斥），可以在枚举下方加上
/// ```
/// LLAMA_ENABLE_BITWISE_OPS( 枚举类型 );
/// ```
/// 
/// 这使枚举支持以下位运算符：
/// `~` `&` `|` `^` `&=` `|=` `^=`
/// 以及额外两个运算符用来将枚举转为bool类型。
/// `+`（一元） `!`
/// 
/// enum class 不能隐式转换为 bool ，用 `+` 让它显式转为bool。例如：
/// ```
/// if(+(flag & MyOption::MyFlag))
/// ```
/// 等价于
/// ```
/// if(static_cast<bool>(flag & MyOption::MyFlag))
/// ```

#pragma once
#include "enum_bitwise_ops.h"
#include <cstdint>
#include <type_traits>

namespace llama
{

/// 决定如何将字符串转为路径
enum class ReadPathOption : uint32_t
{
    None = 0x0000'0000,
    /// 开：允许路径中包含 `..`;
    /// 关：不允许路径中包含 `..`
    AllowDotDot = 0x0000'0001,
};
LLAMA_ENABLE_BITWISE_OPS(ReadPathOption);

/// 决定如何将路径转为字符串
enum class WritePathOption : uint32_t
{
    None = 0x0000'0000,
    /// 开：分隔符为 `\` (Windows 分隔符);
    /// 关：分隔符为 `/` (Linux 分隔符).
    WindowsSep = 0x0000'0001,
};

constexpr WritePathOption WritePathOption_NativeSep =
#ifdef LLAMA_WIN
    WritePathOption::WindowsSep
#else
    WritePWritePathOption::None
#endif
    ;
LLAMA_ENABLE_BITWISE_OPS(WritePathOption);

enum class ExceptionKind : uint32_t
{
    // 通用
    NullPointer,
    BadArgument,

    // 容器相关
    IndexOutofRange,
    ElementDoesNotExist,
    ElementAlreadyExists,

    // 文件系统
    InvalidRelativePath,

    // 字符串
    InvalidByteSequence
};

// 表示协程的三种状态
enum class PromiseStatus : uint32_t
{
    // 协程仍未执行到 final_suspend
    NotDone,
    // 协程结束,抛出了一个异常
    HasException,
    // 协程结束,并成功给出了返回值
    HasResult,
};
} // namespace llama