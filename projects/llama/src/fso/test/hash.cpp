// #include "dog.pb.h"
// #include "base/codec.h"
// #include "base/exceptions.h"
// #include <codecvt>
// #include <cstdint>
// #include <gtest/gtest.h>
// #include <locale>
// #include <string>
// #include <string_view>
// #include <vector>
// #include "fso/hash.h"
// using namespace llama;

// TEST(HashTest, ToString1)
// {
//     Hash hash(1, 1);
//     std::string hs = hash.ToString();
//     EXPECT_EQ(hs, "00000000000000010000000000000001");
// }

// TEST(HashTest, ToString2)
// {
//     Hash hash(0, 0);
//     std::string hs = hash.ToString();
//     EXPECT_EQ(hs, "00000000000000000000000000000000");
// }

// TEST(HashTest, ToString3)
// {
//     Hash hash(0x019f3eacf1e0dd5a, 0x1af3fec47701ceb6);
//     std::string hs = hash.ToString();
//     EXPECT_EQ(hs, "019f3eacf1e0dd5a1af3fec47701ceb6");
// }

// TEST(HashTest, ToString4)
// {
//     Hash hash1("019f3eacf1e0dd5a1af3fec47701ceb6");
//     Hash hash2(0x019f3eacf1e0dd5a, 0x1af3fec47701ceb6);

//     EXPECT_EQ(hash1, hash2);
//     EXPECT_EQ(hash1.ToString(), hash2.ToString());
// }