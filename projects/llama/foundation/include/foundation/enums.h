/// @file
/// llama的所有枚举类型都定义在这里。以便拓展。
///
/// @warning **枚举类型** 应遵循如下规则：
/// 1. 禁用无作用域枚举（ `enum` ），用作用域枚举（ `enum class` ）代替
/// 2. 必须指定枚举基类型。（例： `enum class A : uint32_t` ）
///
/// 如果枚举类型支持位运算，可以在枚举下方加上
/// ```
/// LLAMA_ENABLE_BITWISE_OPS( 枚举类型 );
/// ```
///
/// 这使枚举支持以下位运算符：
/// `~` `&` `|` `^` `&=` `|=` `^=`
/// 以及额外两个运算符用来将枚举转为bool类型。
/// 一元 `+` 和 `!`
///
/// C++的 enum class 不能隐式转换为 bool ，用 `+` 让它显式转为bool。例如：
/// ```
/// if(+(flag & MyOption::MyFlag))
/// ```
/// 等价于
/// ```
/// if(static_cast<bool>(flag & MyOption::MyFlag))
/// ```
///
/// @warning **支持位运算的枚举** 应遵循如下规则：
/// 1. 除了零值 `None` 以外，仅定义值为 2ⁿ 的枚举项。即，枚举项的值只定义 `=1` `=2` `=4` `=8` 的，而不定义 `=3` `=12`
/// 这种的。
/// 2. 如果有些位的组合不属于 2ⁿ ，但又比较常用。可以作为常量定义在枚举外面。命名规则： `枚举类型名_常用值名` ，例如
/// ::WritePathOption_NativeSep
///
#pragma once
#include "enum_bitwise_ops.h"
#include <cstdint>
#include <type_traits>

namespace llama
{

/// 决定如何将字符串转为路径
enum class ReadPathOption : uint32_t
{
    None = 0,
    /// @arg 该位为 `1` - 允许路径中包含 `..`
    /// @arg 该位为 `0` - 不允许路径中包含 `..`
    AllowDotDot = 1,
};
LLAMA_ENABLE_BITWISE_OPS(ReadPathOption);

/// 决定如何将路径转为字符串。
enum class WritePathOption : uint32_t
{
    None = 0,
    /// @arg 该位为 `1` - 输出的分隔符为 `\` (Windows 分隔符)
    /// @arg 该位为 `0` - 输出的分隔符为 `/` (Linux 分隔符)
    ///
    /// ::WritePathOption_NativeSep - 根据目标平台自动决定这一位。
    WindowsSep = 1,
};

/// 根据当前操作系统，该常量被定义为 `WritePathOption::WindowsSep` (Windows) 或 `WritePathOption::None` (非 Windows)
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