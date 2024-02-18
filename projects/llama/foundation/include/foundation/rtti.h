#pragma once

#include <cstdint>
#include <map>
#include <set>
#include <string>

namespace llama
{

enum class ObjectKind : uint32_t
{
    None = 0,
    Object = 1,

    /// 大于等于该数值，表示该值为动态分配而来，见 RttiContext::GetObjectKind 。
    Dynamic = 0x80000000,
};

class RttiContext
{
  public:
    using CastFunction = void *(*)(void *) noexcept;

    ObjectKind GetObjectKind(const char *name)
    {
        const ObjectKind nextKind = static_cast<ObjectKind>(m_nextObjectKind);
        const auto insert = m_strToObjectKind.insert({name, nextKind});
        const bool insert_success = insert.second;
        const auto iter = insert.first;
        if (insert_success)
        {
            m_nextObjectKind++;
            return nextKind;
        }
        else // already exists
        {
            return iter->second;
        }
    }

    ObjectKind GetObjectKind(const void *key)
    {
        const ObjectKind nextKind = static_cast<ObjectKind>(m_nextObjectKind);
        const auto insert = m_ptrToObjectKind.insert({key, nextKind});
        const bool insert_success = insert.second;
        const auto iter = insert.first;
        if (insert_success)
        {
            m_nextObjectKind++;
            return nextKind;
        }
        else // already exists
        {
            return iter->second;
        }
    }

    bool AddCast(ObjectKind src, ObjectKind dst, CastFunction fn)
    {
        auto &&dst_map = m_casts[src];
        return dst_map.insert({dst, fn}).second;
    }

    void *Cast(void *src_obj, ObjectKind src, ObjectKind dst)
    {
        if (void *result = DirectCast(src_obj, src, dst))
        {
            return result;
        }
        std::set<ObjectKind> blacklist;
        if (void *result = RecursiveCast(src_obj, src, dst, blacklist))
        {
            return result;
        }
        return nullptr;
    }

    const void *Cast(const void *src_obj, ObjectKind src, ObjectKind dst)
    {
        return Cast(const_cast<void *>(src_obj), src, dst);
    }

  private:
    void *DirectCast(void *src_obj, ObjectKind src, ObjectKind dst)
    {
        auto src_iter = m_casts.find(src);
        if (src_iter != m_casts.end())
        {
            auto &&dst_to_fn = src_iter->second;
            auto dst_iter = dst_to_fn.find(dst);
            if (dst_iter != dst_to_fn.end())
            {
                auto cast_fn = dst_iter->second;
                return cast_fn(src_obj);
            }
        }
        return nullptr;
    }

    void *RecursiveCast(void *src_obj, ObjectKind src, ObjectKind dst, std::set<ObjectKind> &blacklist)
    {
        auto src_iter = m_casts.find(src);
        if (src_iter != m_casts.end())
        {
            auto &&dst_to_fn_map = src_iter->second;
            // for bridge in src.children:
            //    try cast: src -> bridge -> dst
            for (auto &&pair : dst_to_fn_map)
            {
                const ObjectKind bridge = pair.first;

                if (blacklist.count(bridge))
                    continue;

                auto src_to_bridge_fn = pair.second;
                void *bridge_obj = src_to_bridge_fn(src_obj);
                if (bridge_obj == nullptr)
                {
                    blacklist.insert(bridge);
                    continue;
                }

                void *dst_obj = RecursiveCast(bridge_obj, bridge, dst, blacklist);
                if (dst_obj)
                {
                    return dst_obj;
                }
                else
                {
                    blacklist.insert(bridge);
                    continue;
                }
            }
        }
        return nullptr;
    }

  private:
    uint32_t m_nextObjectKind = (uint32_t)ObjectKind::Dynamic;
    std::map<std::string, ObjectKind> m_strToObjectKind;
    std::map<const void *, ObjectKind> m_ptrToObjectKind;
    /// m_casts[src][dst] -> cast_fn
    std::map<ObjectKind, std::map<ObjectKind, CastFunction>> m_casts;
};

/// 定义类型 T 到 ObjectKind 枚举值的映射关系
/// 需要自行特化。
/// 例：
/// ```
/// template <>
/// class type_to_object_kind<MyObject>
/// {
///   public:
///     static constexpr ObjectKind value = ObjectKind::MyObject;
/// };
/// ```
template <typename T> class type_to_object_kind;

} // namespace llama