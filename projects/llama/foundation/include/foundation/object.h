#pragma once
#include "foundation/enums.h"
#include "foundation/exceptions.h"
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
#define LLAMA_OBJ
namespace llama
{

class IMetaInfo
{
  public:
    virtual ~IMetaInfo() = default;

  private:
    virtual ObjectKind GetObjectKind_IMetaInfo() const = 0;
    virtual void *GetTopLevelObject_IMetaInfo() const = 0;

  public:
    ObjectKind GetObjectKind() const
    {
        return GetObjectKind_IMetaInfo();
    }

    const void *GetTopLevelObject() const
    {
        return GetTopLevelObject_IMetaInfo();
    }

    void *GetTopLevelObject()
    {
        return GetTopLevelObject_IMetaInfo();
    }

    void *Cast(RttiContext &context, ObjectKind dst)
    {
        return context.Cast(GetTopLevelObject(), GetObjectKind(), dst);
    }

    const void *Cast(RttiContext &context, ObjectKind dst) const
    {
        return context.Cast(GetTopLevelObject(), GetObjectKind(), dst);
    }

    template <typename Dst> Dst *Cast(RttiContext &context)
    {
        return static_cast<Dst *>(context.Cast(GetTopLevelObject(), GetObjectKind(), type_to_object_kind<Dst>::value));
    }

    template <typename Dst> const Dst *Cast(RttiContext &context) const
    {
        return static_cast<const Dst *>(
            context.Cast(GetTopLevelObject(), GetObjectKind(), type_to_object_kind<Dst>::value));
    }
};

class BasicMetaInfo : public IMetaInfo
{
    LLAMA_OBJ
  public:
    BasicMetaInfo(RttiContext *context, ObjectKind kind, void *topLvlObj)
        : m_context(context), m_kind(kind), m_topLvlObj(topLvlObj)
    {
    }

    void *Cast(ObjectKind dst)
    {
        return IMetaInfo::Cast(*m_context, dst);
    }

    const void *Cast(ObjectKind dst) const
    {
        return IMetaInfo::Cast(*m_context, dst);
    }

    template <typename Dst> Dst *Cast()
    {
        return IMetaInfo::Cast<Dst>(*m_context);
    }

    template <typename Dst> const Dst *Cast() const
    {
        return IMetaInfo::Cast<Dst>(*m_context);
    }

  private:
    ObjectKind GetObjectKind_IMetaInfo() const override
    {
        return m_kind;
    }

    void *GetTopLevelObject_IMetaInfo() const override
    {
        return m_topLvlObj;
    }

  private:
    void *m_topLvlObj;
    RttiContext *m_context;
    ObjectKind m_kind;
};

class Object : public BasicMetaInfo
{
  public:
    virtual ~Object() = default;
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