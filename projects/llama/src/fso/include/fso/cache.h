#include "base/base.h"
#include "base/lru_cache.h"
#include "fso/deserializer.h"
#include "fso/interface/fso.h"
#include "fso/serializer.h"
#include <cstddef>
#include <fstream>
namespace llama
{
namespace fso
{

/// @brief IFso 对象的键值对数据库。
/// @details 使用上，类似从 Hash 键映射到 IFso 值的 map 。区别是
/// 存放（ ICache::Get ）时不需要指定键类型 Hash ，因为 Hash 可以直接从值类型 IFso 算出来。
/// 存放后会将 IFso 对象的哈希返回出来。
/// 用 ICache::Put 可以将之前存入的对象取回。
/// 重写了 lru_cache 的虚函数，用磁盘做二级缓存：
/// 在溢出时将 IFso 序列化并写入文件。在找不到 key 时，会从文件反序列化出一个 IFso 。
/// 请确保存入的对象具有 LLAMA_RTTI 。否则将无法反序列化。
/// 和 DelayedPointer 一起使用，实现无感知的对象懒加载。
class Cache : public virtual ICache, private lru_cache<Hash, sp<IFso>>
{

public:
    explicit Cache(p<RttiContext> ctx, size_t max_size) : lru_cache<Hash, sp<IFso>>(max_size), m_context(ctx)
    {
    }

private:
    virtual Hash Put_fso_ICache(sp<IFso> obj) override
    {
        Hash h = obj->GetHash();
        this->put(h, obj);
        return h;
    }

    virtual sp<IFso> Get_fso_ICache(const Hash &hash) override
    {
        return this->get(hash);
    }

    virtual void overflow(const Hash &key, const sp<IFso> &value) override
    {
        std::string fileName = HashToString(key);
        fileName += ".bin";
        /// 可以假定 key 和 value.GetHash() 确实一致
        std::ofstream file(fileName, std::ios::binary);
        Serializer s(&file);
        s.Execute(*m_context, *value);
    }

    virtual sp<IFso> miss(const Hash &key) override
    {
        std::string fileName = HashToString(key);
        fileName += ".bin";
        std::ifstream file(fileName, std::ios::binary);
        up<IFso> p = Deserializer(&file).Execute(*m_context);
        return p;
    }

    p<RttiContext> m_context;
};

} // namespace fso
} // namespace llama