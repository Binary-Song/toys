#pragma once
#include "foundation/enums.h"
#include "foundation/exceptions.h"
#include "foundation/macros.h"
#include "foundation/object.h"
#include "foundation/pointers.h"
#include "rtti.h"
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
#include <type_traits>

#define LLAMA_RTTI_UPCAST_BEGIN(T)                                                                                     \
  private:                                                                                                             \
    friend class rtti_trait<T>;                                                                                        \
                                                                                                                       \
  private:                                                                                                             \
    inline static char s_typeId[] = #T;                                                                                \
                                                                                                                       \
  private:                                                                                                             \
    inline static unsigned char s_add_default_cast = []() {

#define LLAMA_RTTI_UPCAST_ENTRY(From, To) ::llama::AddDefaultCast<From, To>();

#define LLAMA_RTTI_UPCAST_END                                                                                          \
    return (unsigned char)0;                                                                                           \
    }                                                                                                                  \
    ();

#define LLAMA_RTTI_OVERRIDE_FUNCS(T)                                                                                   \
  private:                                                                                                             \
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

template <typename From, typename To> inline void AddDefaultCast()
{
    RttiContext::AddDefaultCast(rtti_trait<From>::id, rtti_trait<To>::id,
                                [](void *from) noexcept -> void * { return static_cast<To *>((From *)from); });
}

class IRtti
{
  public:
    virtual ~IRtti() = default;

  private:
    virtual const void *GetSelf_IRtti() const = 0;
    virtual TypeId GetTypeId_IRtti() const = 0;

  public:
    void *GetSelf()
    {
        return const_cast<void *>(const_cast<const IRtti *>(this)->GetSelf_IRtti());
    }

    const void *GetSelf() const
    {
        return GetSelf_IRtti();
    }

    TypeId GetTypeId() const
    {
        return GetTypeId_IRtti();
    }
};

class Object : public virtual IRtti
{
    LLAMA_RTTI(Object)

  public:
    virtual ~Object() = default;

    explicit Object(RttiContext *context) : m_context(context)
    {
    }

    template <typename Dst> Dst *Cast()
    {
        return (Dst *)m_context->Cast(GetSelf(), GetTypeId(), rtti_trait<Dst>::id);
    }

  private:
    RttiContext *m_context;
};

// /// 对象缓存。可能会将对象放在缓存或内存里。
// class ObjectStore
// {
//   public:
//     /// TODO: 改成 Path
//     explicit ObjectStore(std::string tmp_dir) : m_tmp_dir(std::move(tmp_dir))
//     {
//     }

//     /// 存放 `object` 。
//     /// @return 对象的哈希
//     /// @exception 如果对象 `object` 已经存在，将抛出 `ExceptionKind::ElementAlreadyExists`
//     Hash Store(sp<Object> object)
//     {
//         Hash key = object->HashAsObject();
//         auto lower_bound = m_cache.lower_bound(key);
//         if (lower_bound != m_cache.end())
//         {
//             auto less = m_cache.key_comp();
//             // 即 key >= lower bound
//             // 而根据定义 lower bound <= key
//             // 故 key == lower bound
//             if (!less(key, lower_bound->first))
//             {
//                 throw Exception{ExceptionKind::ElementAlreadyExists};
//             }
//         }
//         else
//         {
//             m_cache.insert(lower_bound, {key, object});
//         }
//         return key;
//     }

//     /// 获取哈希值为 `hash` 的对象 `T` 。
//     /// @tparam `T` 必须为 `Object` 的子类
//     /// @exception 如果 哈希值为 `hash` 的对象 不存在，将抛出 `ExceptionKind::ElementDoesNotExist`
//     template <typename T> sp<T> Retrieve(Hash const &hash, std::function<T(std::istream &)> const &consumer)
//     {
//         auto iter = m_cache.find(hash);
//         if (iter != m_cache.end())
//         {
//             sp<Object> object = iter->second;
//             sp<T> t = std::static_pointer_cast<T>(object);
//             return t;
//         }
//         // look for it in the cache files
//         // TODO: use my apis
//         const std::string path = m_tmp_dir + "/" + hash.ToString();

//         if (!std::filesystem::exists(path))
//             throw Exception{ExceptionKind::ElementDoesNotExist};

//         std::ifstream file(path);
//         if (!file)
//             throw Exception{ExceptionKind::ElementDoesNotExist};

//         T obj = consumer(file);
//         sp<T> shared_obj = std::make_shared(std::move(obj));
//         m_cache[hash] = obj;

//         return obj;
//     }

//   private:
//     std::string m_tmp_dir;
//     std::map<Hash, sp<Object>> m_cache;
// };

} // namespace llama