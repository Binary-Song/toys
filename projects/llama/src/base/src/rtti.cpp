#include "base/rtti.h"
#include "base/interfaces/rtti.h"

namespace llama
{

RttiContext &RttiContext::GetDefaultInstance()
{
    static RttiContext context{false};
    return context;
}

up<IRtti> RttiContext::Instantiate(TypeId typeId)
{
    try
    {
        auto iter = m_instantiators.find(typeId);
        if (iter != m_instantiators.end())
        {
            auto ptr = iter->second();
            if (!ptr)
                throw Exception("failed to do RTTI instantiate");
            return up<IRtti>(ptr);
        }
        throw Exception("failed to do RTTI instantiate");
    }
    catch (...)
    {
        throw Exception("failed to do RTTI instantiate");
    }
}
} // namespace llama
