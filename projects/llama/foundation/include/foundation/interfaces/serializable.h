#pragma once
#include "../hash.h"

namespace llama
{

class ISerializable
{
public:
    virtual ~ISerializable() = default;

private:
    virtual void Serialize_ISerializable(std::ostream &out) const = 0;
    virtual void Deserialize_ISerializable(std::istream &in) = 0;

public:
    void Serialize(std::ostream &out) const
    {
        return Serialize_ISerializable(out);
    }

    void Deserialize(std::istream &in)
    {
        return Deserialize_ISerializable(in);
    }
};

} // namespace llama