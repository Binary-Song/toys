#pragma once
#include "foundation/exceptions.h"
#include "hash.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <type_traits>

namespace llama
{
namespace fso
{

class IHasher
{
public:
    virtual ~IHasher() = default;

private:
    void Add();
};

/// 可读可写的一块buffer。
class IRWBuffer
{
public:
    virtual ~IRWBuffer() = default;

public:
    std::tuple<const char *, size_t> Get() const
    {
        return Get_IProperty();
    }

    void Set(const char *data, size_t size)
    {
        return Set_IProperty(data, size);
    }

private:
    /// 返回 buffer 的指针和长度。用于读取。
    virtual std::tuple<const char *, size_t> Get_IProperty() const = 0;
    /// 将长度为 sz 的数据 data 拷贝到 buffer 里。
    /// 可以 sz 重新分配一块合适大小的 buffer。
    virtual void Set_IProperty(const char *data, size_t sz) = 0;
};

class IFso;

class IFsoVisitor
{
public:
    virtual ~IFsoVisitor() = default;

public:
    /// 访问简单属性（数字、字符串等）
    void Visit(up<IRWBuffer> buffer)
    {
        return Visit_ITreeVisitor(std::move(buffer));
    }

    /// 访问固定大小子对象。
    void Visit(IFso &sub)
    {
        Visit_ITreeVisitor(sub);
    }

	/// 访问动态大小子对象
    void Visit()
    {
    }

private:
    virtual void Visit_ITreeVisitor(up<IRWBuffer> buffer) = 0;
    virtual void Visit_ITreeVisitor(IFso &sub) = 0;
};

class IFso
{
public:
    virtual ~IFso() = default;

public:
    void Accept(IFsoVisitor &visitor) const
    {
        return Accept_IFso(visitor);
    }

    Hash GetHash() const
    {
        return GetHash_IFso();
    }

    bool IsDirty() const
    {
        return IsDirty_IFso();
    }

private:
    /// 调用 IFsoVisitor::Visit 来让 Visitor 知道你有什么成员
    virtual void Accept_IFso(IFsoVisitor &visitor) const = 0;
    /// 计算哈希。
    virtual Hash GetHash_IFso() const = 0;
    /// 返回当前对象是否为脏。脏表示本对象的哈希值可能变化，需要重新计算。包含子对象的，不必考虑子对象。
    virtual bool IsDirty_IFso() const = 0;
};

////////////////////////////////
//            I M P L
////////////////////////////////

class FixedSizeRWBuffer : public virtual IRWBuffer
{
public:
    template <typename T, typename = std::enable_if<std::is_arithmetic<T>::value, void>::type>
    explicit FixedSizeRWBuffer(T &buffer) : m_data(&buffer), m_size(sizeof(T))
    {
    }

    FixedSizeRWBuffer(const char *data, size_t size) : m_data(data), m_size(size)
    {
    }

private:
    virtual std::tuple<const char *, size_t> Get_IProperty() const override
    {
        return {m_data, m_size};
    }

    virtual void Set_IProperty(const char *data, size_t sz) override
    {
        if (sz != m_size)
            throw Exception{"Incorrect length."};
        std::copy((const char *)data, (const char *)data + sz, (char *)m_data);
    }

    const char *m_data;
    size_t m_size;
};

class VectorRWBuffer : public virtual IRWBuffer
{
public:
    explicit VectorRWBuffer(std::string &buffer)
        : m_get_buffer([&buffer]() -> char * { return (char *)buffer.data(); }),
          m_get_size([&buffer]() -> size_t { return buffer.size(); }),
          m_resize([&buffer](size_t new_size) -> void { buffer.resize(new_size); })
    {
    }

    explicit VectorRWBuffer(std::vector<char> &buffer)
        : m_get_buffer([&buffer]() -> char * { return (char *)buffer.data(); }),
          m_get_size([&buffer]() -> size_t { return buffer.size(); }),
          m_resize([&buffer](size_t new_size) -> void { buffer.resize(new_size); })
    {
    }

    explicit VectorRWBuffer(std::vector<unsigned char> &buffer)
        : m_get_buffer([&buffer]() -> char * { return (char *)buffer.data(); }),
          m_get_size([&buffer]() -> size_t { return buffer.size(); }),
          m_resize([&buffer](size_t new_size) -> void { buffer.resize(new_size); })
    {
    }

private:
    virtual std::tuple<const char *, size_t> Get_IProperty() const override
    {
        return {m_get_buffer(), m_get_size()};
    }

    virtual void Set_IProperty(const char *data, size_t sz) override
    {
        m_resize(sz);
        char *buff = m_get_buffer();
        std::copy((const char *)data, (const char *)data + sz, (char *)buff);
    }

    std::function<char *()> m_get_buffer;
    std::function<size_t()> m_get_size;
    std::function<void(size_t)> m_resize;
};

class FsoDeserializer : public virtual IFsoVisitor
{
private:
    virtual void Visit_ITreeVisitor(up<IRWBuffer> buffer)
    {
    }

    virtual void Visit_ITreeVisitor(IFso &sub)
    {
    }
};

} // namespace fso
} // namespace llama