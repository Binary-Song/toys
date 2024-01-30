#pragma once
#include "foundation/exceptions.h"
#include "foundation/pointers.h"
#include <cassert>
#include <cstdint>
#include <iostream>
#include <map>
#include <ostream>
#include <string>

namespace llama
{

class Hash
{
  public:
    constexpr Hash(uint64_t data1, uint64_t data2) : m_data1{data1}, m_data2(data2)
    {
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

/// 对象仓库。
class ObjectStore
{
  public:
    ObjectStore()
    {
    }

    /// 存放 `object` 。
    /// @return 对象的哈希
    /// @exception 如果对象已经存在，将抛出 `ExceptionKind::ElementAlreadyExists`
    Hash Store(sp<Object> object)
    {
        Hash key = object->HashAsObject();
        auto lower_bound = m_cache.lower_bound(key);
        if (lower_bound != m_cache.end())
        {
            auto less = m_cache.key_comp();
            if (!less(key, lower_bound->first))
            // 即 key >= lower bound
            // 而根据定义 lower bound <= key
            // 故 key == lower bound
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
    /// @tparam T 必须为 `Object` 的子类
    /// @exception Exception
    template <typename T> sp<T> Retrieve(Hash const &hash)
    {
        auto iter = m_cache.find(hash);
        if (iter != m_cache.end())
        {
            return iter->second;
        }
    }

  private:
    std::map<Hash, sp<Object>> m_cache;
};

} // namespace llama