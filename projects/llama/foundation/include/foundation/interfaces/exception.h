#pragma once
namespace llama
{

class IException
{

public:
    virtual ~IException() = default;

private:
    virtual const char *GetMessage_IException() const = 0;

public:
    const char *GetMessage() const
    {
        return GetMessage_IException();
    }
};

} // namespace llama