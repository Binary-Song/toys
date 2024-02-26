#pragma once

#include "base/exceptions.h"
#include "base/interfaces/debug_print.h"
#include "base/pointers.h"
#include "base/rtti.h"
#include "misc.h"
#include <fmt/format.h>
#include <map>
#include <set>
#include <string_view>
#include <type_traits>

// ================
// LLAMA_RTTI =
//          LLAMA_RTTI_UPCAST_BEGIN +
//      n * LLAMA_RTTI_UPCAST_ENTRY +
//          LLAMA_RTTI_UPCAST_END
// ================

#define LLAMA_RTTI_UPCAST_BEGIN(T)                                                                                     \
private:                                                                                                               \
    friend class ::llama::rtti_trait<T>;                                                                               \
                                                                                                                       \
private:                                                                                                               \
    inline static constexpr char s_typeId[] = #T;                                                                      \
    static_assert(                                                                                                     \
        LLAMA_RTTI_ArgCheck(#T),                                                                                       \
        "All args to LLAMA_RTTI must be fully-qualified and should not contain any spaces (e.g. ::llama::MyClass )");  \
                                                                                                                       \
private:                                                                                                               \
    inline static unsigned char s_auto_init_rtti = []() {

#define LLAMA_RTTI_UPCAST_ENTRY(From, To)                                                                              \
    ::llama::ModifyDefaultRtti<From, To>();                                                                            \
    static_assert(                                                                                                     \
        LLAMA_RTTI_ArgCheck(#To),                                                                                      \
        "All args to LLAMA_RTTI must be fully-qualified and should not contain any spaces (e.g. ::llama::MyClass "     \
        ")");

#define LLAMA_RTTI_UPCAST_END                                                                                          \
    return (unsigned char)0;                                                                                           \
    }                                                                                                                  \
    ();

#define LLAMA_RTTI_OVERRIDE_FUNCS(T)                                                                                   \
private:                                                                                                               \
    const void *GetSelf_IRtti() const override                                                                         \
    {                                                                                                                  \
        static_assert(std::is_same<std::decay<decltype(*this)>::type, T>::value,                                       \
                      "The 1st arg to LLAMA_RTTI must be the current type.");                                          \
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

class IRtti;

/// 保存RTTI信息，并提供类型转换实现。
/// @note 如果需要进行类型转换，建议用 IRtti 接口提供的类型安全 API 。
/// RttiContext 具有一个全局单例，本类提供的静态 (函数名带 "Default" 的) 方法能向这个单例注册各种信息。
/// RttiContext 在创建时，默认拷贝此全局单例。全局单例修改，其他已有的 RttiContext 对象不会
/// 收到通知或受到任何影响。
///
/// @note 不要自己创建 Context 对象，需要时应将其参数化或保存其指针。RttiContext 的作者认为将来会有一个更大的 Context
/// 去继承或者包含 这个类型。要用时就传 Context ，最终目的是除了极少数情况之外，消除全局变量/单例。这样创建新的 Context
/// 程序就能从干净的状态重新执行，有利于单测。 目前本类型属于这个“极少数情况”，因为不这样的话注册 RTTI
/// 将变的格外复杂，这种做法是 RttiContext 的特例，不可作为惯例。
class RttiContext : public virtual IDebugPrint
{
public:
    using CastFunction = void *(*)(void *) noexcept;
    using Instantiator = IRtti *(*)();

    explicit RttiContext(bool fromDefaultInstance = true)
    {
        if (fromDefaultInstance)
        {
            *this = GetDefaultInstance();
        }
    }

    static bool AddDefaultInstantiator(TypeId src, Instantiator fn)
    {
        return GetDefaultInstance().AddInstantiator(src, fn);
    }

    bool AddInstantiator(TypeId src, Instantiator fn)
    {
        return m_instantiators.insert({src, fn}).second;
    }

    /// @brief 注册继承关系
    /// 这些信息会在 RttiContext 对象创建时写入该对象。
    static bool AddDefaultCast(TypeId src, TypeId dst, CastFunction fn)
    {
        return GetDefaultInstance().AddCast(src, dst, fn);
    }

    /// @copybrief AddDefaultCast
    /// 为当前对象添加 RTTI 信息。
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

    LLAMA_API(base) up<IRtti> Instantiate(TypeId typeId);

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
    virtual void DebugPrint_IDebugPrint(std::ostream &stream) const
    {
        stream << fmt::format("{} cast edge(s) :\n", m_casts.size());
        for (auto &&kv : m_casts)
        {
            auto &&map = kv.second;
            for (auto &&kv2 : map)
            {
                stream << fmt::format("  {} -> {}\n", kv.first, kv2.first);
            }
        }
        stream << fmt::format("{} instantiator(s) :\n", m_casts.size());
        for (auto &&kv : m_instantiators)
        {
            stream << fmt::format("  {} \n", kv.first);
        }
    }

private:
    /// m_casts[src][dst] -> cast_fn
    std::map<TypeId, std::map<TypeId, CastFunction>> m_casts;
    std::map<TypeId, Instantiator> m_instantiators;

    LLAMA_API(base) static RttiContext &GetDefaultInstance();
};

template <typename T> class rtti_trait
{
public:
    inline static constexpr const char *id = &T::s_typeId[0];
};

template <typename From, typename To> inline void ModifyDefaultRtti()
{
    RttiContext::AddDefaultCast(rtti_trait<From>::id, rtti_trait<To>::id,
                                [](void *from) noexcept -> void * { return static_cast<To *>((From *)from); });
    AddDefaultInstantiatorIfDefaultConstructible<From>();
}

template <typename T>
inline typename std::enable_if<!std::is_default_constructible<T>::value, void>::type
AddDefaultInstantiatorIfDefaultConstructible()
{
}

template <typename T>
inline typename std::enable_if<std::is_default_constructible<T>::value, void>::type
AddDefaultInstantiatorIfDefaultConstructible()
{
    RttiContext::AddDefaultInstantiator(rtti_trait<T>::id, []() -> IRtti * { return new T{}; });
}

inline constexpr bool LLAMA_RTTI_ArgCheck(const char *arg)
{
    if (arg[0] != ':')
        return false;

    if (arg[1] != ':')
        return false;

    while (*arg)
    {
        char c = *arg;

        if (c == ' ' || c == '\t' || c == '\n')
            return false;

        arg++;
    }
    return true;
}

} // namespace llama