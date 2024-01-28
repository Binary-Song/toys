#pragma once
#include "enums.h"
namespace llama
{


class Exception final
{
  public:
    explicit Exception(ExceptionKind kind) : m_kind(kind), m_message("")
    {
    }

    /// 指定消息 `message` 构建异常。
    /// @note 异常对象不会深拷贝 `message` 。为了确保正常，应指定一个编译期常量。
    explicit Exception(ExceptionKind kind, const char *message) : m_kind(kind), m_message(message)
    {
    }

    ExceptionKind Kind() const
    {
        return m_kind;
    }

    const char *Message() const
    {
        return m_message;
    }

  private:
    ExceptionKind m_kind;
    const char *m_message;
};
} // namespace llama