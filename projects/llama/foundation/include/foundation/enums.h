/// @file
/// llama的所有枚举类型都定义在这里。以便拓展。
///
/// 如果枚举类型支持位运算（各枚举项非互斥），可以在枚举下方加上
/// ```
/// LLAMA_ENABLE_BITWISE_OPS( 枚举类型 );
/// ```

#pragma once
#include "enum_bitwise_ops.h"
#include <cstdint>
#include <type_traits>

namespace llama
{

enum class ReadPathOptions : uint32_t
{
    None = 0x0000'0000,
    AllowDotDot = 0x0000'0001,
};

LLAMA_ENABLE_BITWISE_OPS(ReadPathOptions);

/// 决定将路径转为字符串的行为。
enum class WritePathOptions : uint32_t
{
    None = 0x0000'0000,
    /// 路径分隔符 `\` (Windows 分隔符)
    WindowsSep = 0x0000'0001,
};
LLAMA_ENABLE_BITWISE_OPS(WritePathOptions);

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