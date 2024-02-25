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