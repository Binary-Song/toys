#pragma once
#include "foundation/interfaces/hashable.h"
#include "foundation/lru_cache.h"
#include "foundation/misc.h"
#include <fmt/format.h>
#include <iostream>
#include <istream>
#include <ostream>

namespace llama
{

/// 可以序列化/反序列化的类型。
/// 派生类除了要实现虚函数外，一般还要支持默认构造。
class ISerializable
{
public:
    virtual ~ISerializable() = default;

private:
    virtual void Serialize(std::ostream &out) const = 0;
    virtual void Deserialize(std::istream &in) = 0;
};

class ObjectCache : private lru_cache<Hash, mp<ISerializable, IHashable>>
{
public:


private:
    virtual bool overflow(const Hash &key, const mp<ISerializable, IHashable> &value) override
    {
        
    }

    virtual mp<ISerializable, IHashable> underflow(const Hash &key) override
    {
        
    }
};

} // namespace llama

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
