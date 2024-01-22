#pragma once
namespace llama
{
class Exception
{
public:
	virtual ~Exception() = default;
};

class NullPointerException : public Exception
{};

}//namespace llama