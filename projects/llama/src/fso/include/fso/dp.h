#pragma once
#include "base/pointers.h"
#include "base/rtti.h"
#include "fso/hash.h"
#include "fso/interface/fso.h"
#include <type_traits>

namespace llama
{

class ICache;

namespace fso
{
/// Delay-pointer延迟加载指针：指向的对象只在需要的时候才从缓存读取。
/// @tparam T 必须可以默认构造。因为只有可默认构造的类型才会在 RTTI 注册实例化函数。见 RttiContext 。
template <typename T, typename = std::enable_if<std::is_default_constructible<T>::value, void>::type 
          >
class DelayedPointer
{
public:
    explicit DelayedPointer() : m_hash()
    {
    }

    explicit DelayedPointer(Hash hash) : m_hash(std::move(hash))
    {
    }

    sp<T> Lock(mp<ICache, RttiContext> ctx) const
    {
        sp<IFso> object = std::get<0>(ctx)->Get(m_hash);
        p<T> subobject = object->Cast<T>(*std::get<1>(ctx));
        return sp<T>{object, subobject.data()}; // 特殊的构造函数，新的sp共享计数且指向子对象
    }

    const Hash &GetHashRef() const
    {
        return m_hash;
    }

    Hash &GetHashRef()
    {
        return m_hash;
    }

private:
    Hash m_hash;
};

} // namespace fso

template <typename T> using dp = fso::DelayedPointer<T>;

} // namespace llama