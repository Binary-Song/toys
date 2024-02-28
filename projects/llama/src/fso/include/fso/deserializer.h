#pragma once
#include "base/exceptions.h"
#include "base/rtti.h"
#include "interface/fso.h"
#include <cstdint>
#include <istream>
#include <string>

namespace llama
{

namespace fso
{

class Deserializer : public virtual IVisitor
{
public:
    explicit Deserializer(p<std::istream> stream) : m_stream(stream)
    {
    }

    LLAMA_API(fso) up<IFso> Execute();

    static void DeserializeProperty(IProperty &prop, std::istream &stream)
    {
        uint32_t size = 0;
        for (int i = 0; i < 4; i++)
        {
            if (!stream)
                throw DecodeFailure();
            char ch;
            stream.get(ch);
            if (!stream)
                throw DecodeFailure();
            uint32_t byte = (uint8_t)ch;
            byte <<= i;
            size += (uint32_t)byte;
        }
        if (size == 0)
            throw DecodeFailure();
        std::string str(size, '\0');
        stream.read((char *)str.data(), str.size());
        if (!stream)
            throw DecodeFailure();
        prop.SetValue(str);
    }

private:
    static Exception DecodeFailure()
    {
        return Exception("decode failure");
    }

    virtual void Visit_fso_IVisitor(IProperty &prop) override
    {
        return DeserializeProperty(prop, *m_stream);
    }

    p<std::istream> m_stream;
};

} // namespace fso
} // namespace llama