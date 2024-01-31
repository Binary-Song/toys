#pragma once
#include "foundation/enums.h"
#include "foundation/exceptions.h"
#include "foundation/pointers.h"
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
            throw Exception{ExceptionKind::InvalidByteSequence};
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
                    throw Exception{ExceptionKind::InvalidByteSequence};
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

/// 对象。特点是能被对象仓库管理。
class Object
{
  public:
    virtual ~Object() = default;
    virtual Hash HashAsObject() const = 0;
    virtual void SerializeAsObject(std::ostream &) const = 0;
};

/// 对象缓存。可能会将对象放在缓存或内存里。
class ObjectStore
{
  public:
    /// TODO: 改成 Path
    explicit ObjectStore(std::string tmp_dir) : m_tmp_dir(std::move(tmp_dir))
    {
    }

    /// 存放 `object` 。
    /// @return 对象的哈希
    /// @exception 如果对象 `object` 已经存在，将抛出 `ExceptionKind::ElementAlreadyExists`
    Hash Store(sp<Object> object)
    {
        Hash key = object->HashAsObject();
        auto lower_bound = m_cache.lower_bound(key);
        if (lower_bound != m_cache.end())
        {
            auto less = m_cache.key_comp();
            // 即 key >= lower bound
            // 而根据定义 lower bound <= key
            // 故 key == lower bound
            if (!less(key, lower_bound->first))
            {
                throw Exception{ExceptionKind::ElementAlreadyExists};
            }
        }
        else
        {
            m_cache.insert(lower_bound, {key, object});
        }
        return key;
    }

    /// 获取哈希值为 `hash` 的对象 `T` 。
    /// @tparam `T` 必须为 `Object` 的子类
    /// @exception 如果 哈希值为 `hash` 的对象 不存在，将抛出 `ExceptionKind::ElementDoesNotExist`
    template <typename T> sp<T> Retrieve(Hash const &hash, std::function<T(std::istream &)> const &consumer)
    {
        auto iter = m_cache.find(hash);
        if (iter != m_cache.end())
        {
            sp<Object> object = iter->second;
            sp<T> t = std::static_pointer_cast<T>(object);
            return t;
        }
        // look for it in the cache files
        // TODO: use my apis
        const std::string path = m_tmp_dir + "/" + hash.ToString();

        if (!std::filesystem::exists(path))
            throw Exception{ExceptionKind::ElementDoesNotExist};

        std::ifstream file(path);
        if (!file)
            throw Exception{ExceptionKind::ElementDoesNotExist};

        T obj = consumer(file);
        sp<T> shared_obj = std::make_shared(std::move(obj));
        m_cache[hash] = obj;

        return obj;
    }

  private:
    std::string m_tmp_dir;
    std::map<Hash, sp<Object>> m_cache;
};

} // namespace llama