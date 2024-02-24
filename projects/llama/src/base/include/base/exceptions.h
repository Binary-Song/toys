#pragma once
#include "interfaces/exception.h"
#include <string>
namespace llama
{

class Exception : public virtual IException
{
public:
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