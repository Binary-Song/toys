#pragma once
#include "../rtti.h"
namespace llama
{

/// @brief 表示该类型的对象具有"运行时类型信息"(rtti)。
/// @details 
/// 
/// @note 关于 LLAMA 风格 RTTI 的说明，参见 [RTTI](#rtti) 。
/// 
/// 实现本接口的作用是让对象支持向下转型（基类转派生类）。
/// 
/// 一般不需要自己实现这个接口。使用 LLAMA_RTTI 来自动实现。
/// LLAMA_RTTI 的语法是：
///
/// 1. LLAMA_RTTI(当前类型) 或
/// 2. LLAMA_RTTI(当前类型, 基类1 , ..., 基类n )
///
/// LLAMA_RTTI 需要在 `class` 声明内调用。当前类型就是 LLAMA_RTTI 所在的类型。
///
/// 基类可以是直接基类或者间接基类。 这些基类必须：
///
/// 1. 直接或间接派生自IRtti
/// 2. 用 LLAMA_RTTI 添加了元信息。
///
/// @note 如果直接派生自 IRtti ，不要在 LLAMA_RTTI 内指定 IRtti 。
///
/// 调用 IRtti::Cast 时，当前对象会沿着 LLAMA_RTTI 指定的继承路径进行搜索，
/// 直到找到目标基类。
/// 
/// 由于存在开销，大部分类型不包含 RTTI ，可根据需要添加。
/// 增加 LLAMA_RTTI 不会影响对象的大小和结构、不会增加 dll 的导出符号。
///
class IRtti
{
public:
    virtual ~IRtti() = default;

private:
    /// 返回的指针的类型为 “最派生” 且标记了 LLAMA_RTTI 的类型。
    /// @note 不要手动实现本函数，用 LLAMA_RTTI 。
    virtual const void *GetCanonicalAddress_IRtti() const = 0;

    /// 应返回 “最派生” 且标记了 LLAMA_RTTI 的类型ID。
    /// @note 不要手动实现本函数，用 LLAMA_RTTI 。
    virtual TypeId GetTypeId_IRtti() const = 0;

public:
    void *GetCanonicalAddress()
    {
        return const_cast<void *>(const_cast<const IRtti *>(this)->GetCanonicalAddress_IRtti());
    }

    const void *GetCanonicalAddress() const
    {
        return GetCanonicalAddress_IRtti();
    }

    TypeId GetTypeId() const
    {
        return GetTypeId_IRtti();
    }

    /// 利用 LLAMA_RTTI 进行动态类型转换。支持 upcast 、 downcast 和 cross-cast 。
    /// @exception 如果转换失败则抛出异常。
    template <typename Dst> p<Dst> Cast()
    {
        return RttiContext::GetDefaultInstance()
            .Cast(GetCanonicalAddress(), GetTypeId(), rtti_trait<Dst>::id)
            .unwrap()
            .template static_as<Dst>();
    }

    /// 利用 LLAMA_RTTI 进行动态类型转换。支持 upcast 、 downcast 和 cross-cast
    /// @exception 如果转换失败则抛出异常。
    template <typename Dst> p<const Dst> Cast() const
    {
        return RttiContext::GetDefaultInstance()
            .Cast(GetCanonicalAddress(), GetTypeId(), rtti_trait<Dst>::id)
            .unwrap()
            .template static_as<Dst>();
    }

    /// 利用 LLAMA_RTTI 进行动态类型转换。支持 upcast 、 downcast 和 cross-cast
    /// 如果转换失败返回空指针。
    template <typename Dst> np<Dst> TryCast()
    {
        return RttiContext::GetDefaultInstance()
            .Cast(GetCanonicalAddress(), GetTypeId(), rtti_trait<Dst>::id)
            .template static_as<Dst>();
    }

    /// 利用 LLAMA_RTTI 进行动态类型转换。支持 upcast 、 downcast 和 cross-cast
    /// 如果转换失败返回空指针。
    template <typename Dst> np<const Dst> TryCast() const
    {
        return RttiContext::GetDefaultInstance()
            .Cast(GetCanonicalAddress(), GetTypeId(), rtti_trait<Dst>::id)
            .template static_as<Dst>();
    }
};

} // namespace llama