#include "dog.pb.h"
#include "foundation/codec.h"
#include "foundation/exceptions.h"
#include <codecvt>
#include <cstdint>
#include <gtest/gtest.h>
#include <locale>
#include <string>
#include <string_view>
#include <vector>

using namespace llama;

// TEST(ProtoTest, Test1)
// {
//     Dog dog;
//     dog.set_age(2);
//     dog.set_name("Cat");

//     std::string output;
//     dog.SerializeToString(&output);

//     Dog dog2;
//     dog2.ParseFromString(output);
//     EXPECT_EQ(dog2.name(), "Ca5t");
//     EXPECT_EQ(dog2.age(), 2);
// }
