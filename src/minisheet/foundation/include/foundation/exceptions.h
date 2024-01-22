#pragma once
namespace minisheet
{
class Exception
{
public:
	virtual ~Exception() = default;
};

class NullPointerException : public Exception
{};

}//namespace minisheet