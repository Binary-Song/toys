#pragma once
#include "../hash.h"
#include <string>

namespace llama
{
namespace fso
{

class IReadOnlyProperty
{
public:
    virtual ~IReadOnlyProperty() = default;

    std::string GetValue() const
    {
        return Get_fso_IProperty();
    }

private:
    /// 返回 buffer 的指针和长度。用于读取。
    virtual std::string Get_fso_IProperty() const = 0;
};

class IProperty : public virtual IReadOnlyProperty
{
public:
    void SetValue(std::string_view data)
    {
        return Set_fso_IProperty(data);
    }

private:
    /// 将长度为 sz 的数据 data 拷贝到 buffer 里。
    /// 可以 sz 重新分配一块合适大小的 buffer。
    virtual void Set_fso_IProperty(std::string_view data) = 0;
};

class IFso;

class IVisitor
{
public:
    virtual ~IVisitor() = default;

public:
    void Visit(IProperty &buffer)
    {
        return Visit_fso_IVisitor(buffer);
    }

    void Visit(IProperty &&buffer)
    {
        return Visit_fso_IVisitor(buffer);
    }

private:
    virtual void Visit_fso_IVisitor(IProperty &buffer) = 0;
};

/// 实现类必须有 RTTI ，否则无法反序列化。
class IFso : public virtual IRtti
{
    LLAMA_RTTI(::llama::fso::IFso)

public:
    virtual ~IFso() = default;

public:
    void Accept(IVisitor &visitor)
    {
        return Accept_fso_IFso(visitor);
    }

    LLAMA_API(fso) Hash GetHash();

    // bool IsDirty() const
    // {
    //     return IsDirty_fso_IFso();
    // }

private:
    /// 调用 IFsoVisitor::Visit 来让 Visitor 知道你有什么成员
    virtual void Accept_fso_IFso(IVisitor &visitor) = 0;
    // /// 返回当前对象是否为脏。脏表示本对象的哈希值可能变化，需要重新计算。包含子对象的，不必考虑子对象。
    // virtual bool IsDirty_fso_IFso() const = 0;
};

/// 见 Cache。
class ICache
{
public:
    virtual ~ICache() = default;

    sp<IFso> Get(const Hash &hash)
    {
        return Get_fso_ICache(hash);
    }

    Hash Put(sp<IFso> obj)
    {
        return Put_fso_ICache(std::move(obj));
    }

private:
    virtual sp<IFso> Get_fso_ICache(const Hash &hash) = 0;
    virtual Hash Put_fso_ICache(sp<IFso> obj) = 0;
};

} // namespace fso
} // namespace llama
