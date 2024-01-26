#include "foundation.h"
#include <gtest/gtest.h>
#include <string>

using llama::np;
using llama::p;

class PointerTest : public testing::Test
{};

TEST_F(PointerTest, NP1)
{
	int i = 1;
	int j = 2;
	np<int> p = &i;
	ASSERT_EQ(p, &i);
	p = &j;
	ASSERT_EQ(p, &j);
	ASSERT_EQ(*p.unwrap(), 2);
}

TEST_F(PointerTest, NP2)
{
	np<int> p = nullptr;
	EXPECT_THROW({ p.unwrap(); }, llama::NullPointerException);
}

TEST_F(PointerTest, NP3)
{
	int *a = nullptr;
	np<int> p = a;
	EXPECT_THROW({ p.unwrap(); }, llama::NullPointerException);
}

TEST_F(PointerTest, P1)
{
	std::string x = "123";
	p<std::string> p = &x;
	EXPECT_EQ(p->at(0), '1');
	EXPECT_EQ(p->at(1), '2');

	EXPECT_EQ(&(*p), &x);
	EXPECT_EQ((*p).at(0), '1');
}

TEST_F(PointerTest, P2)
{
	EXPECT_THROW( 
		{
			std::string s;
			p<std::string> ptr = &s;
			ptr = (std::string *)nullptr;
		},
		llama::NullPointerException);
}
     
TEST_F(PointerTest, P3)
{
	EXPECT_THROW(
		{
			std::string *sp = nullptr;
			p<std::string> p = sp;
		},
		llama::NullPointerException);
	EXPECT_THROW({ np<int>(nullptr).deref(); }, llama::NullPointerException);
	EXPECT_THROW({ auto p = np<std::string>(nullptr)->data(); }, llama::NullPointerException);
	EXPECT_THROW({ np<std::string>(nullptr)[0]; }, llama::NullPointerException);
}

struct A
{
	int a;
};

struct B : A
{
	std::string b;
};

struct C : A
{
	double c;
};

struct D : B, C
{
	float d;
};

TEST_F(PointerTest, InheritanceCastsFromRaw)
{
	D d;
	p<A> pa = static_cast<B *>(&d);
	A *a = static_cast<B *>(&d);
	EXPECT_EQ(pa, a);
}

TEST_F(PointerTest, InheritanceCastsFromRaw2)
{
	D d;
	np<A> pa = static_cast<B *>(&d);
	A *a = static_cast<B *>(&d);
	EXPECT_EQ(pa, a);
}

TEST_F(PointerTest, ConstCastsFromRaw)
{
	D d;
	p<const D> pcd = &d;
	p<D> pd = &d;
	EXPECT_EQ(pcd, pd);
}

TEST_F(PointerTest, Arith)
{
	int x[] = {1, 2, 3, 4, 5};
	p<int> p1(x);
	EXPECT_EQ(p1.deref(), 1);
	EXPECT_EQ((p1 + 1).deref(), 2);
	EXPECT_EQ((p1 + 2).deref(), 3);
	EXPECT_EQ((p1 + 4).deref(), 5);
	EXPECT_GT((p1 + 4), p1);
	EXPECT_LT((p1 + 1), p1 + 2);
}