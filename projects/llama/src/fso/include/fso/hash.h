#pragma once
#include "base/exceptions.h"
#include "base/pointers.h"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fmt/format.h>
#include <fstream>
#include <functional>
#include <iostream>
#include <istream>
#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <string_view>
namespace llama
{

class Hash
{
  public:
    /// 构造哈希。
    /// @param data1 高64位
    /// @param data2 低64位
    constexpr Hash(uint64_t data1, uint64_t data2) : m_data1{data1}, m_data2(data2)
    {
    }

    /// 构造哈希。
    /// @param str 代表哈希值的16进制字符串。必须包含恰好32个字符，只能包含 0-9 A-Z a-z。
    /// 这个串的解释方法和数学的顺序一样：index小的、写在前面的数位更大，index大的、写在后面的数位更小。
    constexpr Hash(std::string_view str)
    {
        if (str.size() != 32)
        {
            throw Exception{"Invalid byte sequence."};
        }
        auto fill_data = [](const char *str, uint64_t &data, size_t begin) {
            data = 0;
            for (size_t c = begin; c < begin + 16; c++)
            {
                data *= 16;
                const char ch = str[c];
                if (ch >= '0' && ch <= '9')
                    data += (ch - '0');
                else if (ch >= 'a' && ch <= 'f')
                    data += (ch - 'a' + 10);
                else if (ch >= 'A' && ch <= 'F')
                    data += (ch - 'A' + 10);
                else
                    throw Exception{"Invalid byte sequence."};
            }
        };
        fill_data(str.data(), m_data1, 0);
        fill_data(str.data(), m_data2, 16);
    }

    /// 转为小写字符串。
    std::string ToString() const
    {
        return fmt::format("{0:016x}{1:016x}", m_data1, m_data2);
    }

    constexpr bool operator==(Hash const &other) const
    {
        return m_data1 == other.m_data1 && m_data2 == other.m_data2;
    }

    constexpr bool operator!=(Hash const &other) const
    {
        return m_data1 != other.m_data1 || m_data2 != other.m_data2;
    }

    constexpr bool operator<(Hash const &other) const
    {
        if (m_data1 < other.m_data1)
            return true;
        else if (m_data1 > other.m_data1)
            return false;
        else
            return (m_data2 < other.m_data2);
    }

    constexpr bool operator>(Hash const &other) const
    {
        if (m_data1 > other.m_data1)
            return true;
        else if (m_data1 < other.m_data1)
            return false;
        else
            return (m_data2 > other.m_data2);
    }

    constexpr bool operator<=(Hash const &other) const
    {
        if (m_data1 < other.m_data1)
            return true;
        else if (m_data1 > other.m_data1)
            return false;
        else
            return (m_data2 <= other.m_data2);
    }

    constexpr bool operator>=(Hash const &other) const
    {
        if (m_data1 > other.m_data1)
            return true;
        else if (m_data1 < other.m_data1)
            return false;
        else
            return (m_data2 >= other.m_data2);
    }

  private:
    uint64_t m_data1;
    uint64_t m_data2;
};

} // namespace llama