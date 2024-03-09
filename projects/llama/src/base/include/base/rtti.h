#pragma once

#include "base/interfaces/debug_print.h"
#include "base/pointers.h"
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
        ::llama::detail::RttiModifier::LLAMA_RTTI_ArgCheck(#T),                                                        \
        "All args to LLAMA_RTTI must be fully-qualified and should not contain any spaces (e.g. ::llama::MyClass )");  \
                                                                                                                       \
private:                                                                                                               \
    inline static unsigned char s_auto_init_rtti = []() {

#define LLAMA_RTTI_UPCAST_ENTRY(From, To)                                                                              \
    ::llama::detail::RttiModifier::ModifyDefaultRtti<From, To>();                                                      \
    static_assert(                                                                                                     \
        ::llama::detail::RttiModifier::LLAMA_RTTI_ArgCheck(#To),                                                       \
        "All args to LLAMA_RTTI must be fully-qualified and should not contain any spaces (e.g. ::llama::MyClass "     \
        ")");

#define LLAMA_RTTI_UPCAST_END                                                                                          \
    return (unsigned char)0;                                                                                           \
    }                                                                                                                  \
    ();

#define LLAMA_RTTI_OVERRIDE_FUNCS(T)                                                                                   \
private:                                                                                                               \
    const void *GetCanonicalAddress_IRtti() const override                                                                         \
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

/// llama 的 API
namespace llama
{

/// detail 命名空间里的所有成员都属于实现细节。不可使用。
namespace detail
{
class RttiModifier;
}

/// 类型ID，一个静态的字符串。比较时比较其内容而不是地址。
using TypeId = std::string_view;

class IRtti;

/// 保存RTTI数据（继承图），并提供类型转换实现。
/// @note 关于 LLAMA 风格 RTTI 的说明，参见 [RTTI](#rtti) 。
/// @sa llama::IRtti [RTTI](#rtti)
class RttiContext : public virtual IDebugPrint
{
    friend class detail::RttiModifier;

public:
    using CastFunction = void *(*)(void *) noexcept;
    using Instantiator = IRtti *(*)();

    /// 返回单例。
    LLAMA_API(base) static RttiContext &GetDefaultInstance();

    /// 将动态类型为 src 的 src_obj 转换为 dst 类型，src 和 dst 都是类型 id 。
    /// @warning 如果需要进行类型转换，建议用 IRtti 接口提供的类型安全 API 来替代。
    LLAMA_API(base) np<void> Cast(void *src_obj, TypeId src, TypeId dst);

    /// 将动态类型为 src 的 src_obj 转换为 dst 类型，src 和 dst 都是类型 id 。
    /// 常量重载。
    /// @warning 如果需要进行类型转换，建议用 IRtti 接口提供的类型安全 API 来替代。
    np<const void> Cast(const void *src_obj, TypeId src, TypeId dst)
    {
        return Cast(const_cast<void *>(src_obj), src, dst);
    }

    /// 指定类型，构造其实例。
    LLAMA_API(base) up<IRtti> Instantiate(TypeId typeId);

private:
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

    static bool AddDefaultCast(TypeId src, TypeId dst, CastFunction fn)
    {
        return GetDefaultInstance().AddCast(src, dst, fn);
    }

    bool AddCast(TypeId src, TypeId dst, CastFunction fn)
    {
        auto &&dst_map = m_casts[src];
        return dst_map.insert({dst, fn}).second;
    }

private:
    LLAMA_API(base) void *DirectCast(void *src_obj, TypeId src, TypeId dst);
    LLAMA_API(base) void *RecursiveCast(void *src_obj, TypeId src, TypeId dst, std::set<TypeId> &blacklist);

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
};

template <typename T> class rtti_trait
{
public:
    inline static constexpr const char *id = &T::s_typeId[0];
};
namespace detail
{

/// Rtti 修改器。里面装着永远都别碰的危险黑科技。
class RttiModifier
{
public:
    friend class ::llama::RttiContext;

    template <typename T>
    static inline typename std::enable_if<!std::is_default_constructible<T>::value, void>::type
    AddDefaultInstantiatorIfDefaultConstructible()
    {
    }

    template <typename T>
    static inline typename std::enable_if<std::is_default_constructible<T>::value, void>::type
    AddDefaultInstantiatorIfDefaultConstructible()
    {
        RttiContext::AddDefaultInstantiator(rtti_trait<T>::id, []() -> IRtti * { return new T{}; });
    }

    template <typename From, typename To> static inline void ModifyDefaultRtti()
    {
        static_assert(std::is_base_of<To, From>::value, "The 1st argument to LLAMA_RTTI should derive publicly from the other arguments.");
        RttiContext::AddDefaultCast(rtti_trait<From>::id, rtti_trait<To>::id,
                                    [](void *from) noexcept -> void * { return static_cast<To *>((From *)from); });
        AddDefaultInstantiatorIfDefaultConstructible<From>();
    }

    static inline constexpr bool LLAMA_RTTI_ArgCheck(const char *arg)
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
};

} // namespace detail
} // namespace llama