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

void *RttiContext::DirectCast(void *src_obj, TypeId src, TypeId dst)
{
    if (src == dst)
        return src_obj;
    auto src_iter = m_casts.find(src);
    if (src_iter != m_casts.end())
    {
        auto &&dst_to_fn = src_iter->second;
        auto dst_iter = dst_to_fn.find(dst);
        if (dst_iter != dst_to_fn.end())
        {
            auto cast_fn = dst_iter->second;
            return cast_fn(src_obj);
        }
    }
    return nullptr;
}

void *RttiContext::RecursiveCast(void *src_obj, TypeId src, TypeId dst, std::set<TypeId> &blacklist)
{
    auto src_iter = m_casts.find(src);
    if (src_iter != m_casts.end())
    {
        auto &&dst_to_fn_map = src_iter->second;
        // for bridge in src.children:
        //    try cast: src -> bridge -> dst
        for (auto &&pair : dst_to_fn_map)
        {
            const TypeId bridge = pair.first;

            if (blacklist.count(bridge))
                continue;

            auto src_to_bridge_fn = pair.second;
            void *bridge_obj = src_to_bridge_fn(src_obj);
            if (bridge_obj == nullptr)
            {
                blacklist.insert(bridge);
                continue;
            }

            void *dst_obj = RecursiveCast(bridge_obj, bridge, dst, blacklist);
            if (dst_obj)
            {
                return dst_obj;
            }
            else
            {
                blacklist.insert(bridge);
                continue;
            }
        }
    }
    return nullptr;
}

np<void> RttiContext::Cast(void *src_obj, TypeId src, TypeId dst)
{
    if (void *result = DirectCast(src_obj, src, dst))
    {
        return result;
    }
    std::set<TypeId> blacklist;
    if (void *result = RecursiveCast(src_obj, src, dst, blacklist))
    {
        return result;
    }
    return nullptr;
}
} // namespace llama
