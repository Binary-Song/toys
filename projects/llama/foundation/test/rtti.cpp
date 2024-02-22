#include "foundation/rtti.h"
#include "foundation/hash.h"
#include "foundation/interfaces/hashable.h"
#include "foundation/interfaces/rtti.h"
#include "foundation/misc.h"
#include "foundation/object_cache.h"
#include <gtest/gtest.h>

namespace llama
{

class Fun : public virtual IHashable, public virtual IException
{
private:
    virtual Hash GetHash_IHashable() const
    {
        return Hash(111, 222);
    }
    virtual const char *GetMessage_IException() const
    {
        return "Fun exception";
    }
};

Hash hash(multi<IHashable, IException> obj)
{
    return std::get<0>(obj)->GetHash();
}

std::string except_msg(multi<IHashable, IException> obj)
{
    return std::get<1>(obj)->GetMessage();
}

class GrandParent : public virtual IRtti
{
    LLAMA_RTTI(GrandParent)
};

class Parent : public GrandParent
{
    LLAMA_RTTI(Parent, GrandParent)
};

class Child : public Parent
{
    LLAMA_RTTI(Child, Parent)
};

} // namespace llama

TEST(rtti, case1)
{
    llama::Fun fun;
    auto res = hash(&fun);
    EXPECT_EQ(res, llama::Hash(111, 222));
}

TEST(rtti, case2)
{
    llama::Fun fun;
    auto res = except_msg(&fun);
    EXPECT_EQ(res, "Fun exception");
}

TEST(rtti, case3)
{
    using llama::Child;
    using llama::GrandParent;
    using llama::Parent;
    using llama::RttiContext;

    RttiContext ctx;
    Child child;
    GrandParent &parent = child;
    Child *child2 = parent.Cast<Child>(ctx);
    EXPECT_EQ(&child, child2);
}

TEST(rtti, case4)
{
    llama::Fun fun;
    llama::multi<llama::IHashable, llama::IException> p = &fun;
    llama::multi<llama::IHashable, llama::IException> p1 = p; // 确保我们的构造函数模板不能隐藏拷贝和移动构造。
    llama::multi<llama::IHashable, llama::IException> p2 = std::move(p);

    EXPECT_EQ(std::get<0>(p1), std::get<0>(p2));
    EXPECT_EQ(std::get<1>(p1), std::get<1>(p2));
}