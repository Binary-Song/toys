#include "base/rtti.h"
#include "base/exceptions.h"
#include "base/interfaces/rtti.h"
#include <gtest/gtest.h>
#include <sstream>

namespace llama
{

// class Fun : public virtual IHashable, public virtual IException
// {
// private:
//     virtual Hash GetHash_IHashable() const
//     {
//         return Hash(111, 222);
//     }
//     virtual const char *GetMessage_IException() const
//     {
//         return "Fun exception";
//     }
// };

// Hash hash(mp<IHashable, IException> obj)
// {
//     return std::get<0>(obj)->GetHash();
// }

// std::string except_msg(mp<IHashable, IException> obj)
// {
//     return std::get<1>(obj)->GetMessage();
// }

class GrandParent : public virtual IRtti
{
    LLAMA_RTTI(::llama::GrandParent)

    int data1;
};

class Parent : public GrandParent
{
    LLAMA_RTTI(::llama::Parent, ::llama::GrandParent)
    int data2;
};

class Child : public Parent
{
    LLAMA_RTTI(::llama::Child, ::llama::Parent)
    int data3;
};

class Child2 : public Parent
{
    LLAMA_RTTI(::llama::Child2, ::llama::Parent)
    int data3;
};

class NoDefaultCtor : public virtual IRtti
{
    LLAMA_RTTI(::llama::NoDefaultCtor)

    explicit NoDefaultCtor(int data) : data(data)
    {
    }
    int data;
};

} // namespace llama

// TEST(rtti, case1)
// {
//     llama::Fun fun;
//     auto res = hash(&fun);
//     EXPECT_EQ(res, llama::Hash(111, 222));
// }

// TEST(rtti, case2)
// {
//     llama::Fun fun;
//     auto res = except_msg(&fun);
//     EXPECT_EQ(res, "Fun exception");
// }

TEST(rtti, case3)
{
    using llama::Child;
    using llama::GrandParent;
    using llama::Parent;
    using llama::RttiContext;

    Child child;
    GrandParent &parent = child;
    Child *child2 = parent.Cast<Child>();
    EXPECT_EQ(&child, child2);
    std::stringstream strm;
    RttiContext::GetDefaultInstance().DebugPrint(strm);
    EXPECT_EQ("3 cast edge(s) :\n  ::llama::Child -> ::llama::Parent\n  ::llama::Child2 -> ::llama::Parent\n  "
              "::llama::Parent -> ::llama::GrandParent\n3 instantiator(s) :\n  ::llama::Child \n  ::llama::Child2 \n  "
              "::llama::Parent \n",
              strm.str());
}

// TEST(rtti, case4)
// {
//     llama::Fun fun;
//     llama::mp<llama::IHashable, llama::IException> p = &fun;
//     llama::mp<llama::IHashable, llama::IException> p1 = p; // 确保我们的构造函数模板不能隐藏拷贝和移动构造。
//     llama::mp<llama::IHashable, llama::IException> p2 = std::move(p);

//     EXPECT_EQ(std::get<0>(p1), std::get<0>(p2));
//     EXPECT_EQ(std::get<1>(p1), std::get<1>(p2));
// }

TEST(rtti, case5)
{
    llama::Child2 child2;
    llama::GrandParent &gp = child2;
    EXPECT_THROW({ auto p = gp.Cast<llama::Child>(); }, llama::Exception);
}