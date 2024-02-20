/// @file
/// 字符串编码转换。
/// 需要注意 llama 的所有 API 中， `char*` 、`std::string` 等窄字节字符串一律视为 utf-8

#pragma once

#include "config.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include "macros.h"

namespace llama
{

/// 将 UTF-16 字符串解析成代码点。
/// @exception 如果解析失败，抛出 ExceptionKind::InvalidByteSequence
LLAMA_API(foundation) std::vector<uint32_t> DecodeUtf16(const char16_t *data, size_t length);
/// 将 UTF-8 字符串解析成代码点
LLAMA_API(foundation) std::vector<uint32_t> DecodeUtf8(const char *data, size_t length);

/// 将代码点编码为 UTF-16 字符串
LLAMA_API(foundation) std::u16string EncodeUtf16(const uint32_t *data, size_t length);
/// 将代码点编码为 UTF-8 字符串
LLAMA_API(foundation) std::string EncodeUtf8(const uint32_t *data, size_t length);

inline std::u16string ToUtf16(std::string_view str)
{
    auto codes = DecodeUtf8(str.data(), str.size());
    return EncodeUtf16(codes.data(), codes.size());
}

inline std::u16string ToUtf16(std::wstring_view str)
{
#ifdef LLAMA_WIN
    return std::u16string((const char16_t *)str.data(), str.size());
#else
    return EncodeUtf16((const uint32_t *)str.data(), str.size());
#endif
}

inline std::string ToUtf8(std::u16string_view str)
{
    auto codes = DecodeUtf16(str.data(), str.size());
    return EncodeUtf8(codes.data(), codes.size());
}

inline std::string ToUtf8(std::wstring_view str)
{
#ifdef LLAMA_WIN
    std::u16string_view v((const char16_t *)str.data(), str.size());
    return ToUtf8(v);
#else
    return EncodeUtf8((const uint32_t *)str.data(), str.size());
#endif
}

} // namespace llama