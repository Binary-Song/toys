#pragma once
#include "../hash.h"

namespace llama
{

class IHashable
{
public:
    virtual ~IHashable() = default;

private:
    virtual Hash GetHash_IHashable() const = 0;

public:
    const Hash GetHash() const
    {
        return GetHash_IHashable();
    }
};

} // namespace llama