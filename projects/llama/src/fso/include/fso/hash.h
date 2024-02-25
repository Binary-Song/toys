#pragma once
#include "base/base.h"
#include <array>
#include <cstddef>
namespace llama
{

using Hash = std::array<byte, 20>;

inline std::string HashToString(const Hash &hash)
{
    static constexpr char toHex[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

    std::string str(hash.size() * 2, 0);
    for (size_t i = 0; i < hash.size(); i++)
    {
        str[i * 2] = toHex[hash[i] / 16];
        str[i * 2 + 1] = toHex[hash[i] % 16];
    }
    return str;
}

} // namespace llama