#pragma once

#include <ostream>
namespace llama
{

/// @brief 输出自己的状态供调试用。
class IDebugPrint
{

public:
    virtual ~IDebugPrint() = default;

private:
    /// @brief 实现这个方法，输出调试信息
    virtual void DebugPrint_IDebugPrint(std::ostream &stream) const = 0;

public:
    void DebugPrint(std::ostream &stream) const
    {
        return DebugPrint_IDebugPrint(stream);
    }
};
} // namespace llama