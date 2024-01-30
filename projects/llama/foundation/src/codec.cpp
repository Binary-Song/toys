#include "foundation/codec.h"
#include "foundation/enums.h"
#include "foundation/exceptions.h"

namespace llama
{

LLAMA_FND_API std::vector<uint32_t> DecodeUtf16(const char16_t *data, size_t length)
{
    std::vector<uint32_t> result;

    for (size_t i = 0; i < length;)
    {
        uint32_t code_point;

        // Check if the current code unit is a high surrogate
        if (data[i] >= 0xD800 && data[i] <= 0xDBFF)
        {
            // Check if there are enough remaining code units for a surrogate pair
            if (i + 1 < length && data[i + 1] >= 0xDC00 && data[i + 1] <= 0xDFFF)
            {
                // Combine the high and low surrogates to form a Unicode code point
                code_point = ((data[i] - 0xD800) << 10) + (data[i + 1] - 0xDC00) + 0x10000;
                i += 2; // Consume two code units
            }
            else
            {
                // Invalid surrogate pair, skip the high surrogate alone
                code_point = 0xFFFD; // Replacement character
                i += 1;              // Consume one code unit
                throw Exception(ExceptionKind::InvalidByteSequence);
            }
        }
        else
        {
            // The current code unit is not a high surrogate
            code_point = data[i];
            i += 1; // Consume one code unit
        }

        // Add the decoded code point to the result vector
        result.push_back(code_point);
    }

    return result;
}

LLAMA_FND_API std::vector<uint32_t> DecodeUtf8(const char *data, size_t length)
{
    std::vector<uint32_t> codePoints;
    codePoints.reserve(length); // Reserve memory for the maximum possible code points

    size_t i = 0;
    while (i < length)
    {
        uint32_t codePoint;
        uint8_t ch = data[i];

        if (ch < 0x80)
        {
            // Single-byte character
            codePoint = ch;
            i += 1;
        }
        else if ((ch & 0xE0) == 0xC0)
        {
            // Two-byte character
            codePoint = ((ch & 0x1F) << 6) | (data[i + 1] & 0x3F);
            i += 2;
        }
        else if ((ch & 0xF0) == 0xE0)
        {
            // Three-byte character
            codePoint = ((ch & 0x0F) << 12) | ((data[i + 1] & 0x3F) << 6) | (data[i + 2] & 0x3F);
            i += 3;
        }
        else if ((ch & 0xF8) == 0xF0)
        {
            // Four-byte character
            codePoint =
                ((ch & 0x07) << 18) | ((data[i + 1] & 0x3F) << 12) | ((data[i + 2] & 0x3F) << 6) | (data[i + 3] & 0x3F);
            i += 4;
        }
        else
        {
            // Invalid UTF-8 sequence
            i += 1;
            throw Exception(ExceptionKind::InvalidByteSequence);
        }

        codePoints.push_back(codePoint);
    }

    return codePoints;
}

LLAMA_FND_API std::u16string EncodeUtf16(const uint32_t *data, size_t length)
{
    std::u16string result;
    for (size_t i = 0; i < length; i++)
    {
        uint32_t codepoint = data[i];
        if (codepoint <= 0xFFFF)
        {
            result.push_back(static_cast<char16_t>(codepoint));
        }
        else
        {
            codepoint -= 0x10000;
            char16_t highSurrogate = static_cast<char16_t>((codepoint >> 10) + 0xD800);
            char16_t lowSurrogate = static_cast<char16_t>((codepoint & 0x3FF) + 0xDC00);
            result.push_back(highSurrogate);
            result.push_back(lowSurrogate);
        }
    }
    return result;
}

LLAMA_FND_API std::string EncodeUtf8(const uint32_t *data, size_t length)
{
    std::string encodedString;

    for (size_t i = 0; i < length; i++)
    {
        uint32_t codePoint = data[i];

        if (codePoint <= 0x7F)
        {
            // Single byte character
            encodedString += static_cast<char>(codePoint);
        }
        else if (codePoint <= 0x7FF)
        {
            // Two byte character
            encodedString += static_cast<char>((codePoint >> 6) | 0xC0);
            encodedString += static_cast<char>((codePoint & 0x3F) | 0x80);
        }
        else if (codePoint <= 0xFFFF)
        {
            // Three byte character
            encodedString += static_cast<char>((codePoint >> 12) | 0xE0);
            encodedString += static_cast<char>(((codePoint >> 6) & 0x3F) | 0x80);
            encodedString += static_cast<char>((codePoint & 0x3F) | 0x80);
        }
        else if (codePoint <= 0x10FFFF)
        {
            // Four byte character
            encodedString += static_cast<char>((codePoint >> 18) | 0xF0);
            encodedString += static_cast<char>(((codePoint >> 12) & 0x3F) | 0x80);
            encodedString += static_cast<char>(((codePoint >> 6) & 0x3F) | 0x80);
            encodedString += static_cast<char>((codePoint & 0x3F) | 0x80);
        }
    }

    return encodedString;
}

} // namespace llama
