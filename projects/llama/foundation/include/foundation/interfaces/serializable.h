#pragma once
#include "../hash.h"
#include "foundation/rtti.h"
#include "rtti.h"

namespace llama
{

/// 可以序列化、反序列化的类型。
/// @note 实现本接口的类型必须标注 LLAMA_RTTI 。否则无法正常反序列化。
class ISerializable : public virtual IRtti
{
    LLAMA_RTTI(::llama::ISerializable)

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