#pragma once
#include "base/exceptions.h"
#include "base/misc.h"
#include "interface/fso.h"
#include <cstdint>
#include <limits>
#include <string>

namespace llama
{
namespace fso
{

class Serializer : public virtual IVisitor
{
public:
    explicit Serializer(p<std::ostream> stream) : m_stream(stream)
    {
    }

    LLAMA_API(fso) void Execute( IFso &object);

    static void SerializeProperty(IReadOnlyProperty &prop, std::ostream &stream)
    {
        std::string value = prop.GetValue();
        if (value.size() > std::numeric_limits<uint32_t>::max())
            throw EncodeFailure();
        // todo: Make Async!!! Make Safe!!!
        uint32_t size = (uint32_t)value.size();
        for (int i = 0; i < 4; i++)
        {
            if (!stream)
                throw EncodeFailure();
            uint8_t byte = static_cast<uint8_t>(size & 0xFF);
            stream.put(byte);
            size /= 0x100;
        }
        if (!stream)
            throw EncodeFailure();
        stream.write(value.data(), value.size());
    }

private:
    static Exception EncodeFailure()
    {
        return Exception("encode failure");
    }

    virtual void Visit_fso_IVisitor(IProperty &prop) override
    {
        return SerializeProperty(prop, *m_stream);
    }

    p<std::ostream> m_stream;
};

} // namespace fso
} // namespace llama