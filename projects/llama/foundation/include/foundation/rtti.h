#pragma once

#include "misc.h"
#include <cstdint>
#include <functional>
#include <list>
#include <map>
#include <set>
#include <string>
#include <string_view>

#define LLAMA_RTTI_UPCAST_BEGIN(T)                                                                                     \
private:                                                                                                               \
    friend class rtti_trait<T>;                                                                                        \
                                                                                                                       \
private:                                                                                                               \
    inline static char s_typeId[] = #T;                                                                                \
                                                                                                                       \
private:                                                                                                               \
    inline static unsigned char s_add_default_cast = []() {

#define LLAMA_RTTI_UPCAST_ENTRY(From, To) ::llama::AddDefaultCast<From, To>();

#define LLAMA_RTTI_UPCAST_END                                                                                          \
    return (unsigned char)0;                                                                                           \
    }                                                                                                                  \
    ();

#define LLAMA_RTTI_OVERRIDE_FUNCS(T)                                                                                   \
private:                                                                                                               \
    const void *GetSelf_IRtti() const override                                                                         \
    {                                                                                                                  \
        static_assert(std::is_same<std::decay<decltype(*this)>::type, T>::value,                                       \
                      "first arg to LLAMA_RTTI must be the current type.");                                            \
        return this;                                                                                                   \
    }                                                                                                                  \
                                                                                                                       \
    ::llama::TypeId GetTypeId_IRtti() const override                                                                   \
    {                                                                                                                  \
        return &s_typeId[0];                                                                                           \
    }

/// @copydoc llama::IRtti
#define LLAMA_RTTI(...) LLAMA_VA_SELECT(LLAMA_RTTI, __VA_ARGS__) LLAMA_RTTI_UPCAST_END
// clang-format off
#define LLAMA_RTTI_1(T) LLAMA_RTTI_OVERRIDE_FUNCS(T)  LLAMA_RTTI_UPCAST_BEGIN(T) 
#define LLAMA_RTTI_2(T, B1)                             LLAMA_RTTI_1(T)                             LLAMA_RTTI_UPCAST_ENTRY(T, B1)
#define LLAMA_RTTI_3(T, B1, B2)                         LLAMA_RTTI_2(T, B1)                         LLAMA_RTTI_UPCAST_ENTRY(T, B2)
#define LLAMA_RTTI_4(T, B1, B2, B3)                     LLAMA_RTTI_3(T, B1, B2)                     LLAMA_RTTI_UPCAST_ENTRY(T, B3)
#define LLAMA_RTTI_5(T, B1, B2, B3, B4)                 LLAMA_RTTI_4(T, B1, B2, B3)                 LLAMA_RTTI_UPCAST_ENTRY(T, B4)
#define LLAMA_RTTI_6(T, B1, B2, B3, B4, B5)             LLAMA_RTTI_5(T, B1, B2, B3, B4)             LLAMA_RTTI_UPCAST_ENTRY(T, B5)
#define LLAMA_RTTI_7(T, B1, B2, B3, B4, B5, B6)         LLAMA_RTTI_6(T, B1, B2, B3, B4, B5)         LLAMA_RTTI_UPCAST_ENTRY(T, B6)
#define LLAMA_RTTI_8(T, B1, B2, B3, B4, B5, B6, B7)     LLAMA_RTTI_7(T, B1, B2, B3, B4, B5, B6)     LLAMA_RTTI_UPCAST_ENTRY(T, B7)
#define LLAMA_RTTI_9(T, B1, B2, B3, B4, B5, B6, B7, B8) LLAMA_RTTI_8(T, B1, B2, B3, B4, B5, B6, B7) LLAMA_RTTI_UPCAST_ENTRY(T, B8)
// clang-format on

namespace llama
{

/// 类型ID，一个静态的字符串。比较时比较其内容而不是地址。
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

    np<void> Cast(void *src_obj, TypeId src, TypeId dst)
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

    np<const void> Cast(const void *src_obj, TypeId src, TypeId dst)
    {
        return Cast(const_cast<void *>(src_obj), src, dst);
    }

private:
    void *DirectCast(void *src_obj, TypeId src, TypeId dst)
    {
        if (src == dst)
            return src_obj;
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

template <typename From, typename To> inline void AddDefaultCast()
{
    RttiContext::AddDefaultCast(rtti_trait<From>::id, rtti_trait<To>::id,
                                [](void *from) noexcept -> void * { return static_cast<To *>((From *)from); });
}

} // namespace llama