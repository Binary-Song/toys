#pragma once
namespace llama
{

/// @brief 异常对象。
/// @details 虽然提供了本接口，但仍然不建议抛出非 Exception 的类型。
/// 因为即使加了 -fno-rtti ，catch 和 throw 的类型的 rtti 也会带上。
class IException
{

public:
    virtual ~IException() = default;

private:
    /// 返回异常的说明信息
    virtual const char *GetMessage_IException() const = 0;

public:
    /// 返回异常的说明信息
    const char *GetMessage() const
    {
        return GetMessage_IException();
    }
};

} // namespace llama