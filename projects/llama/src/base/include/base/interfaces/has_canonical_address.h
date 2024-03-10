#pragma once
namespace llama
{

/// @brief 本接口要求获取对象的最外层地址。
class IHasCanonicalAddress
{
public:
    virtual ~IHasCanonicalAddress() = default;

    const void *GetCanonicalAddress() const
    {
        return GetCanonicalAddress_IHasCanonicalAddress();
    }

private:
    /// @brief 正规指针，指对象的最外层地址。获取正规指针的方法是将对象 static_cast 为“最派生类型”的指针。
    virtual const void *GetCanonicalAddress_IHasCanonicalAddress() const = 0;
};
} // namespace llama