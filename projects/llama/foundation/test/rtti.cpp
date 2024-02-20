#include "foundation/rtti.h"
#include "foundation/object.h"
#include <gtest/gtest.h>

namespace llama
{
class Animal : public virtual Object
{
    LLAMA_RTTI(Animal, Object)

  public:
    Animal(RttiContext *context) : Object(context)
    {
    }

    virtual const char *Name()
    {
        return name;
    }

    const char *name = "animal";
};

class Cat : public Animal
{
    LLAMA_RTTI(Cat, Animal, Object)

  public:
    explicit Cat(RttiContext *context) : Animal(context), Object(context)
    {
    }

    virtual const char *Name() override
    {
        return name;
    }

    const char *name = "cat";
};

class Dog : public Animal, virtual Object
{
      LLAMA_RTTI(Dog, Animal, Object)

  public:
    explicit Dog(RttiContext *context) : Animal(context), Object(context)
    {
    }

    virtual const char *Name() override
    {
        return name;
    }

    const char *name = "dog";
};

} // namespace llama

TEST(rtti, case1)
{
    llama::RttiContext c{};
    llama::Dog dog{&c};
    llama::Animal& animal = dog;
    EXPECT_EQ(animal.Cast<llama::Dog>()->Name(), "dog");
}
