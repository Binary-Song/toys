#pragma once

#include <cstdint>
#include <functional>
#include <list>
#include <map>
#include <set>
#include <string>
#include <string_view>

namespace llama
{
using TypeId = std::string_view;
class RttiContext
{
  public:
    using CastFunction = void *(*)(void *) noexcept;

    RttiContext() : m_casts(s_casts)
    {
    }

    static bool AddDefaultCast(TypeId src, TypeId dst, CastFunction fn)
    {
        auto &&dst_map = s_casts[src];
        return dst_map.insert({dst, fn}).second;
    }

    bool AddCast(TypeId src, TypeId dst, CastFunction fn)
    {
        auto &&dst_map = m_casts[src];
        return dst_map.insert({dst, fn}).second;
    }

    void *Cast(void *src_obj, TypeId src, TypeId dst)
    {
        if (void *result = DirectCast(src_obj, src, dst))
        {
            return result;
        }
        std::set<TypeId> blacklist;
        if (void *result = RecursiveCast(src_obj, src, dst, blacklist))
        {
            return result;
        }
        return nullptr;
    }

    const void *Cast(const void *src_obj, TypeId src, TypeId dst)
    {
        return Cast(const_cast<void *>(src_obj), src, dst);
    }

  private:
    void *DirectCast(void *src_obj, TypeId src, TypeId dst)
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

    void *RecursiveCast(void *src_obj, TypeId src, TypeId dst, std::set<TypeId> &blacklist)
    {
        auto src_iter = m_casts.find(src);
        if (src_iter != m_casts.end())
        {
            auto &&dst_to_fn_map = src_iter->second;
            // for bridge in src.children:
            //    try cast: src -> bridge -> dst
            for (auto &&pair : dst_to_fn_map)
            {
                const TypeId bridge = pair.first;

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
    /// m_casts[src][dst] -> cast_fn
    std::map<TypeId, std::map<TypeId, CastFunction>> m_casts;
    LLAMA_API(foundation) inline static std::map<TypeId, std::map<TypeId, CastFunction>> s_casts;
};

template <typename T> class rtti_trait
{
  public:
    inline static constexpr const char *id = &T::s_typeId[0];
};

} // namespace llama