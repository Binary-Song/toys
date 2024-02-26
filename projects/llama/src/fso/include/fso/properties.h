#pragma once
#include "base/exceptions.h"
#include "dp.h"
#include "fso/deserializer.h"
#include "fso/hash.h"
#include "fso/serializer.h"
#include "interface/fso.h"
#include <cstddef>
#include <cstdint>
#include <functional>
#include <sstream>
#include <string>
#include <string_view>

namespace llama
{
namespace fso
{

class NumericProperty : public virtual IProperty
{
public:
    template <typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value, void>::type>
    explicit NumericProperty(T &buffer)
        : m_get([&buffer]() -> double { return static_cast<double>(buffer); }),
          m_set([&buffer](double num) { buffer = static_cast<double>(num); })
    {
    }

private:
    virtual std::string Get_fso_IProperty() const override
    {
        return fmt::format("{}", m_get());
    }

    virtual void Set_fso_IProperty(std::string_view str) override
    {
        std::string s{str};
        size_t pos;
        bool ok = true;
        double num = 0;
        try
        {
            num = std::stod(s, &pos);
        }
        catch (...)
        {
            ok = false;
        }
        if (pos == 0)
            ok = false;

        if (!ok)
        {
            throw Exception{};
        }
        else
        {
            m_set(num);
        }
    }

    std::function<double()> m_get;
    std::function<void(double)> m_set;
};

class StringProperty : public virtual IProperty
{
public:
    explicit StringProperty(std::string &buffer) : m_buffer(&buffer)
    {
    }

private:
    virtual std::string Get_fso_IProperty() const override
    {
        return *m_buffer;
    }

    virtual void Set_fso_IProperty(std::string_view str) override
    {
        m_buffer->assign(str);
    }

    p<std::string> m_buffer;
};

class SubObjectProperty : public virtual IProperty
{
public:
    template <typename T> explicit SubObjectProperty(dp<T> &ptr) : m_hash(&ptr.GetHashRef())
    {
    }

private:
    virtual std::string Get_fso_IProperty() const override
    {
        return std::string((const char *)m_hash->data(), m_hash->size());
    }

    virtual void Set_fso_IProperty(std::string_view str) override
    {
        if (str.size() != m_hash->size())
        {
            throw Exception{};
        }
        std::copy(str.begin(), str.end(), (char *)m_hash->data());
    }

    p<Hash> m_hash;
};

class IVectorProperty : public virtual IProperty
{
private:
    virtual uint32_t Size_IVectorProperty() const = 0;
    virtual void Resize_IVectorProperty(uint32_t size) = 0;
    virtual up<IReadOnlyProperty> ElementAt_IVectorProperty(uint32_t indx) const = 0;
    virtual up<IProperty> ElementAt_IVectorProperty(uint32_t indx) = 0;

private:
    virtual std::string Get_fso_IProperty() const override
    {
        std::stringstream strm;
        uint32_t size = Size_IVectorProperty();
        NumericProperty sizeProp{size};
        Serializer::SerializeProperty(sizeProp, strm);
        for (uint32_t i = 0; i < size; i++)
        {
            up<IReadOnlyProperty> prop = ElementAt_IVectorProperty(i);
            Serializer::SerializeProperty(*prop, strm);
        }
        return strm.str();
    }

    virtual void Set_fso_IProperty(std::string_view str) override
    {
        std::stringstream strm{std::string(str)};
        uint32_t size;
        NumericProperty sizeProp{size};
        Deserializer::DeserializeProperty(sizeProp, strm);

        Resize_IVectorProperty(size);

        for (uint32_t i = 0; i < size; i++)
        {
            up<IProperty> prop = ElementAt_IVectorProperty(i);
            Deserializer::DeserializeProperty(*prop, strm);
        }
    }
};

template <typename T> class DPVectorProperty : public IVectorProperty
{
public:
    explicit DPVectorProperty(std::vector<dp<T>> &vec) : m_vec(&vec)
    {
    }

private:
    virtual uint32_t Size_IVectorProperty() const override
    {
        return m_vec->size();
    }

    virtual void Resize_IVectorProperty(uint32_t size)
    {
        m_vec->resize(size);
    }

    virtual up<IReadOnlyProperty> ElementAt_IVectorProperty(uint32_t indx) const override
    {
        dp<T> &elem = m_vec->at(indx);
        up<IReadOnlyProperty> prop{new SubObjectProperty{elem}};
        return prop;
    }

    virtual up<IProperty> ElementAt_IVectorProperty(uint32_t indx) override
    {
        dp<T> &elem = m_vec->at(indx);
        up<IProperty> prop{new SubObjectProperty{elem}};
        return prop;
    }

    p<std::vector<dp<T>>> m_vec;
};

} // namespace fso
} // namespace llama