#pragma once

#include <ostream>
namespace llama
{
class IDebugPrint
{

public:
    virtual ~IDebugPrint() = default;

private:
    virtual void DebugPrint_IDebugPrint(std::ostream &stream) const = 0;

public:
    void DebugPrint(std::ostream &stream) const
    {
        return DebugPrint_IDebugPrint(stream);
    }
};
} // namespace llama