#include "base/codec.h"
#include "base/exceptions.h"
#include <codecvt>
#include <cstdint>
#include <gtest/gtest.h>
#include <locale>
#include <string>
#include <string_view>
#include <vector>

using namespace llama;

TEST(Utf16Test, DecodeUtf16Basic)
{
    const char16_t input[] = u"Hello, World!";
    std::vector<uint32_t> result = DecodeUtf16(input, sizeof(input) / sizeof(char16_t) - 1);
    EXPECT_EQ(result.size(), 13); // "Hello, World!" 有 13 个字符（包括结束符）
    EXPECT_EQ(result[0], uint32_t('H'));
    EXPECT_EQ(result[1], uint32_t('e'));
    EXPECT_EQ(result[2], uint32_t('l'));
    EXPECT_EQ(result[3], uint32_t('l'));
    EXPECT_EQ(result[4], uint32_t('o'));
    EXPECT_EQ(result[5], uint32_t(','));
    EXPECT_EQ(result[6], uint32_t(' '));
    EXPECT_EQ(result[7], uint32_t('W'));
    EXPECT_EQ(result[8], uint32_t('o'));
    EXPECT_EQ(result[9], uint32_t('r'));
    EXPECT_EQ(result[10], uint32_t('l'));
    EXPECT_EQ(result[11], uint32_t('d'));
    EXPECT_EQ(result[12], uint32_t('!'));
}

TEST(Utf16Test, DecodeUtf16Surrogates)
{
    const char16_t input[] = u"😀"; // Smiling face emoji
    std::vector<uint32_t> result = DecodeUtf16(input, sizeof(input) / sizeof(char16_t) - 1);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], uint32_t(0x1F600));
}

TEST(Utf16Test, DecodeUtf16EmptyString)
{
    const char16_t input[] = u"";
    std::vector<uint32_t> result = DecodeUtf16(input, sizeof(input) / sizeof(char16_t) - 1);
    EXPECT_EQ(result.size(), 0); // 空字符串没有字符

    result = DecodeUtf16(input, sizeof(input) / sizeof(char16_t));
    EXPECT_EQ(result.size(), 1); // 空字符串没有字符
    EXPECT_EQ(result[0], 0);
}

TEST(Utf16Test, DecodeUtf16NullTerminator)
{
    const char16_t input[] = u"Hello";
    std::vector<uint32_t> result = DecodeUtf16(input, sizeof(input) / sizeof(char16_t) - 1);
    EXPECT_EQ(result.size(), 5); // "Hello" 有 5 个字符（包括结束符）
}

TEST(DecodeUtf16, SingleCharacter)
{
    const char16_t data[] = {'A'};
    std::vector<uint32_t> result = DecodeUtf16(data, 1);
    ASSERT_EQ(1, result.size());
    EXPECT_EQ('A', result[0]);
}

TEST(DecodeUtf16, MultipleCharacters)
{
    const char16_t data[] = {'A', 'B', 'C', 'D'};
    std::vector<uint32_t> result = DecodeUtf16(data, 4);
    ASSERT_EQ(4, result.size());
    EXPECT_EQ('A', result[0]);
    EXPECT_EQ('B', result[1]);
    EXPECT_EQ('C', result[2]);
    EXPECT_EQ('D', result[3]);
}

TEST(DecodeUtf16, SurrogatePair)
{
    const char16_t data[] = {0xD83D, 0xDE00}; // Unicode code point for U+1F600 (grinning face emoji)
    std::vector<uint32_t> result = DecodeUtf16(data, 2);
    ASSERT_EQ(1, result.size());
    EXPECT_EQ(0x1F600, result[0]);
}

TEST(DecodeUtf16, InvalidSurrogatePair)
{
    const char16_t data[] = {0xD83D, 'A'};
    EXPECT_THROW({ std::vector<uint32_t> result = DecodeUtf16(data, 2); }, Exception);
}

//////////////////////
// Test case 1: Empty string
TEST(DecodeUtf8Test, EmptyString)
{
    const char *data = "";
    size_t length = 0;
    std::vector<uint32_t> expected = {};
    std::vector<uint32_t> result = DecodeUtf8(data, length);
    EXPECT_EQ(result, expected);
}

// Test case 2: Single-byte characters
TEST(DecodeUtf8Test, SingleByteCharacters)
{
    const char *data = "Hello, world!";
    size_t length = 13;
    std::vector<uint32_t> expected = {'H', 'e', 'l', 'l', 'o', ',', ' ', 'w', 'o', 'r', 'l', 'd', '!'};
    std::vector<uint32_t> result = DecodeUtf8(data, length);
    EXPECT_EQ(result, expected);
}

// Test case 3: Two-byte characters
TEST(DecodeUtf8Test, TwoByteCharacters)
{
    const char *data = "¡Hola, mundo!";
    size_t length = 14;
    std::vector<uint32_t> expected = {0x00A1, 'H', 'o', 'l', 'a', ',', ' ', 'm', 'u', 'n', 'd', 'o', '!'};
    std::vector<uint32_t> result = DecodeUtf8(data, length);
    EXPECT_EQ(result, expected);
}

// Test case 4: Three-byte characters
TEST(DecodeUtf8Test, ThreeByteCharacters)
{
    const char *data = "こんにちは";
    size_t length = 15;
    std::vector<uint32_t> expected = {0x3053, 0x3093, 0x306B, 0x3061, 0x306F};
    std::vector<uint32_t> result = DecodeUtf8(data, length);
    EXPECT_EQ(result, expected);
}

// Test case 5: Four-byte characters
TEST(DecodeUtf8Test, FourByteCharacters)
{
    const char *data = "🌍";
    size_t length = 4;
    std::vector<uint32_t> expected = {0x1F30D};
    std::vector<uint32_t> result = DecodeUtf8(data, length);
    EXPECT_EQ(result, expected);
}

// Test case 6: Invalid UTF-8 sequence
TEST(DecodeUtf8Test, InvalidSequence)
{
    const char *data = "\x80\xC0"; // Invalid UTF-8 sequence
    size_t length = 2;
    std::vector<uint32_t> expected = {};
    EXPECT_THROW({ std::vector<uint32_t> result = DecodeUtf8(data, length); }, Exception);
}

///////////////////////////////

// EncodeUtf16 tests
TEST(EncodeUtf16Test, SingleEmojiCharacter)
{
    const uint32_t emoji = 0x1F600; // Grinning Face emoji
    std::u16string result = EncodeUtf16(&emoji, 1);
    EXPECT_EQ(u"😀", result);
}

TEST(EncodeUtf16Test, MultipleEmojiCharacters)
{
    const uint32_t emojis[] = {0x1F600, 0x1F601,
                               0x1F602}; // Grinning Face, Grinning with Smiling Eyes, and Face with Tears of Joy emojis
    std::u16string result = EncodeUtf16(emojis, 3);
    EXPECT_EQ(u"😀😁😂", result);
}

TEST(EncodeUtf16Test, EmojiFollowedByNonEmojiCharacters)
{
    const uint32_t data[] = {0x1F600, 'A', 'B'}; // Grinning Face emoji, 'A', 'B'
    std::u16string result = EncodeUtf16(data, 3);
    EXPECT_EQ(u"😀AB", result);
}

TEST(EncodeUtf16Test, NonEmojiCharacters)
{
    const uint32_t data[] = {'A', 'B', 'C'};
    std::u16string result = EncodeUtf16(data, 3);
    EXPECT_EQ(u"ABC", result);
}

TEST(EncodeUtf16Test, EmptyInput)
{
    const uint32_t *data = nullptr;
    std::u16string result = EncodeUtf16(data, 0);
    EXPECT_EQ(u"", result);
}

// Test case for encoding a single character
TEST(EncodingTest, SingleCharacter)
{
    uint32_t data[] = {0x24}; // U+0024, dollar sign
    std::string expected = "$";
    std::string actual = EncodeUtf8(data, 1);
    EXPECT_EQ(expected, actual);
}

// Test case for encoding multiple characters
TEST(EncodingTest, MultipleCharacters)
{
    uint32_t data[] = {0x4e16, 0x754c}; // U+4e16 U+754c, "世界", hello world in Chinese
    std::string expected = "\xe4\xb8\x96\xe7\x95\x8c";
    std::string actual = EncodeUtf8(data, 2);
    EXPECT_EQ(expected, actual);
}

// Test case for encoding an empty string
TEST(EncodingTest, EmptyString)
{
    uint32_t data[] = {};
    std::string expected = "";
    std::string actual = EncodeUtf8(data, 0);
    EXPECT_EQ(expected, actual);
}

// Test case for encoding a string with characters outside the BMP
TEST(EncodingTest, CharactersOutsideBMP)
{
    uint32_t data[] = {0x1f600, 0x1f601, 0x1f602}; // U+1f600 U+1f601 U+1f602, emoticons
    std::string expected = "\xf0\x9f\x98\x80\xf0\x9f\x98\x81\xf0\x9f\x98\x82";
    std::string actual = EncodeUtf8(data, 3);
    EXPECT_EQ(expected, actual);
}

TEST(EncodingTest, EmojiCharacters)
{
    uint32_t data[] = {
        0x1f60d, 0x1f618,
        0x1f602}; // U+1f60d U+1f618 U+1f602, smiling face with heart-eyes, face throwing a kiss, face with tears of joy
    std::string expected = "\xf0\x9f\x98\x8d\xf0\x9f\x98\x98\xf0\x9f\x98\x82";
    std::string actual = EncodeUtf8(data, 3);
    EXPECT_EQ(expected, actual);
}