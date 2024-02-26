#pragma once
#include "interfaces/exception.h"
#include <string>
namespace llama
{
    /// IException 的实现类。内部保存异常信息字符串。
class Exception : public virtual IException
{
public:

    /// 构造。会将 message 的副本保存在对象内部。
    explicit Exception(std::string message = "") : m_message(std::move(message))
    {
    }

private:
    virtual const char *GetMessage_IException() const override
    {
        return m_message.c_str();
    }

    std::string m_message;
};

} // namespace llama