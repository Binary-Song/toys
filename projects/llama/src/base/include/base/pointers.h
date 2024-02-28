#pragma once
#include "exceptions.h"
#include <cstddef>
#include <memory>
#include <type_traits>
namespace llama
{

inline Exception NullPointerException()
{
    return Exception("null error");
}

/// @brief 见 llama::p
/// @details 如果你 **真的** 知道一个指针不是空，在 llama::p 的构造函数传这个玩意绕过空检查。
class bypass_null_check
{
};

/// @brief 非空指针。
/// @details 用它来表示已经非空的指针。它没有空的状态，如果试图用空指针初始化它，会
/// 在运行时抛出 NullPointerException 异常。除此之外不会检查空，因此比 llama::np 有更好的性能。
/// 没有默认构造函数。
/// @sa llama::np
template <typename T> class p
{
public:
    /// @brief 禁止用空指针构造。
    p(std::nullptr_t) = delete;

    /// @brief 如果 `U*` 可以转为 `T*` ，则 `U*` 也可以转为 `p<T>`
    /// @exception 要求 `ptr` 非空。否则抛出异常。
    p(T *ptr) : m_ptr(ptr)
    {
        if (!ptr)
            throw NullPointerException();
    }

    /// @brief 如果 `U*` 可以转为 `T*`，则 `p<U>` 也可以转为 `p<T>`
    /// @exception 要求 `ptr` 非空。否则抛出异常。
    template <typename U> p(p<U> ptr) : m_ptr(ptr.as_raw())
    {
        if (!ptr)
            throw NullPointerException();
    }

    /// @brief 绕过空检查，构造。
    /// @details 调用本重载前，请在心中默念十遍， **我真的确定它不是空** 。
    p(T *ptr, bypass_null_check) noexcept : m_ptr(ptr)
    {
    }

    /// @brief 绕过空检查，从其他类型的指针构造。
    /// @details 调用本重载前，请在心中默念十遍， **我真的确定它不是空** 。
    template <typename U> p(p<U> ptr, bypass_null_check) noexcept : m_ptr(ptr.as_raw())
    {
    }

    /// @brief 可以隐式转换为普通指针
    operator T *() const noexcept
    {
        return m_ptr;
    }

    /// @brief 对内部指针进行 static_cast
    template <typename U> p<U> static_as() const noexcept
    {
        return p<U>(static_cast<U *>(m_ptr), bypass_null_check{});
    }

    /// @brief 对内部指针进行 reinterpret_cast
    template <typename U> p<U> reinterpret_as() const noexcept
    {
        return p<U>(reinterpret_cast<U *>(m_ptr), bypass_null_check{});
    }

    /// @brief 对内部指针进行 const_cast
    template <typename U> p<U> const_as() const noexcept
    {
        return p<U>(const_cast<U *>(m_ptr), bypass_null_check{});
    }

    /// 解引用。
    /// 如果 T=void ，无法调用。
    template <typename U = T> U &operator*() const noexcept
    {
        static_assert(!std::is_void<U>::value, "Cannot dereference a void pointer.");
        return *m_ptr;
    }

    /// 解引用。
    /// 如果 T=void ，无法调用。
    template <typename U = T> U &as_ref() const noexcept
    {
        static_assert(!std::is_void<U>::value, "Cannot dereference a void pointer.");
        return *m_ptr;
    }

    /// 返回裸指针。
    T *operator->() const noexcept
    {
        return m_ptr;
    }

    /// 返回裸指针。
    T *as_raw() const noexcept
    {
        return m_ptr;
    }

    /// 访问偏移量。
    /// 如果 T=void ，无效。
    template <typename U = T> U &operator[](ptrdiff_t offset) const noexcept
    {
        static_assert(!std::is_void<U>::value, "Cannot dereference a void pointer.");
        return m_ptr[offset];
    }

    /// 指针算术。
    ptrdiff_t operator-(p<T> p2) const noexcept
    {
        return m_ptr - p2.m_ptr;
    }

    /// 指针算术。
    p<T> operator+(ptrdiff_t offset) const noexcept
    {
        return m_ptr + offset;
    }

    /// 指针算术。
    friend p<T> operator+(ptrdiff_t offset, p<T> p) noexcept
    {
        return p.m_ptr + offset;
    }

    /// 指针算术。
    p<T> operator++(int) noexcept
    {
        p<T> old = *this;
        ++m_ptr;
        return old;
    }

    /// 指针算术。
    p<T> &operator++() noexcept
    {
        ++m_ptr;
        return *this;
    }

    /// 指针算术。
    p<T> operator--(int) noexcept
    {
        p<T> old = *this;
        --m_ptr;
        return old;
    }

    /// 指针算术。
    p<T> &operator--() noexcept
    {
        --m_ptr;
        return *this;
    }

    /// 指针算术。
    p<T> &operator+=(ptrdiff_t offset) noexcept
    {
        m_ptr += offset;
        return *this;
    }

    /// 指针算术。
    p<T> &operator-=(ptrdiff_t offset) noexcept
    {
        m_ptr -= offset;
        return *this;
    }

private:
    T *m_ptr;
};

/// @brief 可空指针。
/// @details 与 llama::p 相反，可以为空。
/// 所有需要解引用的操作都会触发判空，试图对空指针解引用会抛出 llama::Exception 。
/// 如果不希望每次访问都判空，用 unwrap 将其转为 llama::p 即可。
template <typename T> class np
{
public:
    /// @brief 构造空指针
    np() noexcept : ptr(nullptr)
    {
    }

    /// @brief 构造空指针
    np(std::nullptr_t) noexcept : ptr(nullptr)
    {
    }

    /// @brief 转换构造函数，如果 `U*` 可以转为 `T*` ，则 `U*` 也可以转为 `np<T>`
    np(T *ptr) noexcept : ptr(ptr)
    {
    }

    /// @brief 转换构造函数，如果 `U*` 可以转为 `T*` ，则 `np<U>` 也可以转为 `np<T>`
    template <typename U> np(np<U> ptr) noexcept : ptr(ptr.as_raw())
    {
    }

    /// @brief 转换构造函数，如果 `U*` 可以转为 `T*` ，则 `p<U>` 可以转为 `np<T>`
    template <typename U> np(p<U> ptr) noexcept : ptr(ptr.as_raw())
    {
    }

    /// @brief 该函数使 llama::np 可在 if 里作为条件。
    explicit operator bool() const noexcept
    {
        return ptr;
    }

    /// @brief 可以隐式转为普通指针。
    operator T *() const noexcept
    {
        return ptr;
    }

    /// @brief 对内部指针进行 static_cast
    template <typename U> np<U> static_as() const noexcept
    {
        return np<U>(static_cast<U *>(ptr));
    }

    /// @brief 对内部指针进行 reinterpret_cast
    template <typename U> np<U> reinterpret_as() const noexcept
    {
        return np<U>(reinterpret_cast<U *>(ptr));
    }

    /// @brief 对内部指针进行 const_cast
    template <typename U> np<U> const_as() const noexcept
    {
        return np<U>(const_cast<U *>(ptr));
    }

    /// @brief 解引用。
    /// 如果 T=void ，无法调用。
    /// @exception 要求 `ptr` 非空。否则抛出异常。
    template <typename U = T> U &operator*() const
    {
        static_assert(!std::is_void<U>::value, "Cannot dereference a void pointer.");
        if (!ptr)
            throw NullPointerException();
        return *ptr;
    }

    /// @brief 解引用。
    /// 如果 T=void ，无法调用。
    /// @exception 要求 `ptr` 非空。否则抛出异常。
    template <typename U = T> U &as_ref() const
    {
        static_assert(!std::is_void<U>::value, "Cannot dereference a void pointer.");
        if (!ptr)
            throw NullPointerException();
        return *ptr;
    }

    ///  @brief `->` 运算符
    /// @exception 要求 `ptr` 非空。否则抛出异常。
    T *operator->() const
    {
        if (!ptr)
            throw NullPointerException();
        return ptr;
    }

    /// @brief 转为 `p<T>` 。
    /// @exception 要求 `ptr` 非空。否则抛出异常。
    p<T> unwrap() const
    {
        if (!ptr)
            throw NullPointerException();
        return ptr;
    }

    /// @brief 转为 `p<T>` ，如果当前指针为空，返回 subs 作为替代。
    /// @exception 要求 `ptr` 非空。否则抛出异常。
    p<T> unwrap_or(p<T> subs) const noexcept
    {
        if (!ptr)
            return subs;
        return ptr;
    }

    /// @brief 转为裸指针
    T *as_raw() const noexcept
    {
        return ptr;
    }

    /// @brief 当数组一般访问
    /// @exception 要求 `ptr` 非空。否则抛出异常。
    template <typename U = T> U &operator[](ptrdiff_t offset) const
    {
        static_assert(!std::is_void<U>::value, "Cannot dereference a void pointer.");
        if (!ptr)
            throw NullPointerException();
        return ptr[offset];
    }

    /// @brief 指针算术
    ptrdiff_t operator-(np<T> p2) const noexcept
    {
        return ptr - p2.ptr;
    }

    /// @brief 指针算术
    np<T> operator+(ptrdiff_t offset) const noexcept
    {
        return ptr + offset;
    }

    /// @brief 指针算术
    friend np<T> operator+(ptrdiff_t offset, np<T> p) noexcept
    {
        return p.ptr + offset;
    }

    /// @brief 指针算术
    np<T> operator++(int) noexcept
    {
        np<T> old = *this;
        ++ptr;
        return old;
    }

    /// @brief 指针算术
    np<T> &operator++() noexcept
    {
        ++ptr;
        return *this;
    }

    /// @brief 指针算术
    np<T> operator--(int) noexcept
    {
        np<T> old = *this;
        --ptr;
        return old;
    }

    /// @brief 指针算术
    np<T> &operator--() noexcept
    {
        --ptr;
        return *this;
    }

    /// @brief 指针算术
    np<T> &operator+=(ptrdiff_t offset) noexcept
    {
        ptr += offset;
        return *this;
    }

    /// @brief 指针算术
    np<T> &operator-=(ptrdiff_t offset) noexcept
    {
        ptr -= offset;
        return *this;
    }

private:
    T *ptr;
};

// clang-format off
template <typename T, typename U> bool operator==(p<T> p1, p<U> p2) { return static_cast<T *>(p1) == static_cast<U *>(p2); }
template <typename T, typename U> bool operator!=(p<T> p1, p<U> p2) { return static_cast<T *>(p1) != static_cast<U *>(p2); }
template <typename T, typename U> bool operator>=(p<T> p1, p<U> p2) { return static_cast<T *>(p1) >= static_cast<U *>(p2); }
template <typename T, typename U> bool operator<=(p<T> p1, p<U> p2) { return static_cast<T *>(p1) <= static_cast<U *>(p2); }
template <typename T, typename U> bool operator >(p<T> p1, p<U> p2) { return static_cast<T *>(p1)  > static_cast<U *>(p2); }
template <typename T, typename U> bool operator <(p<T> p1, p<U> p2) { return static_cast<T *>(p1)  < static_cast<U *>(p2); }

template <typename T, typename U> bool operator==(p<T> p1, np<U> p2) { return static_cast<T *>(p1) == static_cast<U *>(p2); }
template <typename T, typename U> bool operator!=(p<T> p1, np<U> p2) { return static_cast<T *>(p1) != static_cast<U *>(p2); }
template <typename T, typename U> bool operator>=(p<T> p1, np<U> p2) { return static_cast<T *>(p1) >= static_cast<U *>(p2); }
template <typename T, typename U> bool operator<=(p<T> p1, np<U> p2) { return static_cast<T *>(p1) <= static_cast<U *>(p2); }
template <typename T, typename U> bool operator >(p<T> p1, np<U> p2) { return static_cast<T *>(p1)  > static_cast<U *>(p2); }
template <typename T, typename U> bool operator <(p<T> p1, np<U> p2) { return static_cast<T *>(p1)  < static_cast<U *>(p2); }

template <typename T, typename U> bool operator==(np<T> p1, p<U> p2) { return static_cast<T *>(p1) == static_cast<U *>(p2); }
template <typename T, typename U> bool operator!=(np<T> p1, p<U> p2) { return static_cast<T *>(p1) != static_cast<U *>(p2); }
template <typename T, typename U> bool operator>=(np<T> p1, p<U> p2) { return static_cast<T *>(p1) >= static_cast<U *>(p2); }
template <typename T, typename U> bool operator<=(np<T> p1, p<U> p2) { return static_cast<T *>(p1) <= static_cast<U *>(p2); }
template <typename T, typename U> bool operator >(np<T> p1, p<U> p2) { return static_cast<T *>(p1)  > static_cast<U *>(p2); }
template <typename T, typename U> bool operator <(np<T> p1, p<U> p2) { return static_cast<T *>(p1)  < static_cast<U *>(p2); }

template <typename T, typename U> bool operator==(np<T> p1, np<U> p2) { return static_cast<T *>(p1) == static_cast<U *>(p2); }
template <typename T, typename U> bool operator!=(np<T> p1, np<U> p2) { return static_cast<T *>(p1) != static_cast<U *>(p2); }
template <typename T, typename U> bool operator>=(np<T> p1, np<U> p2) { return static_cast<T *>(p1) >= static_cast<U *>(p2); }
template <typename T, typename U> bool operator<=(np<T> p1, np<U> p2) { return static_cast<T *>(p1) <= static_cast<U *>(p2); }
template <typename T, typename U> bool operator >(np<T> p1, np<U> p2) { return static_cast<T *>(p1)  > static_cast<U *>(p2); }
template <typename T, typename U> bool operator <(np<T> p1, np<U> p2) { return static_cast<T *>(p1)  < static_cast<U *>(p2); }
// clang-format on 

template<typename T>
using sp = std::shared_ptr<T>;

template<typename T>
using up = std::unique_ptr<T>;


/// 多重指针。用来接受同时实现多个接口的对象。
/// 可以用 std::get 访问这些接口。
/// 例：
/// ```
/// void foo(mp<IMyInterface1, IMyInterface2> obj)
/// {
///     p<IMyInterface1> if1 = std::get<0>(obj);
///     p<IMyInterface2> if2 = std::get<1>(obj);
/// }
/// ...
/// MyObject obj; // 假设 MyObject 类实现了 IMyInterface1 和 IMyInterface2
/// foo(&obj);
/// ```
/// 
template <typename... Interfaces> class mp : public std::tuple<p<Interfaces>...>
{
public:
    template <typename Arg, typename U = typename std::enable_if<
                                !std::is_same<typename std::decay<mp>::type, typename std::decay<Arg>::type>::value,
                                void>::type /* 防止隐藏拷贝和移动构造函数 */>
    mp(Arg &&arg) : std::tuple<p<Interfaces>...>{FillWithSame<Arg, p<Interfaces>...>(std::forward<Arg>(arg))}
    {
    } 
 
    ~mp() = default; 
    mp(const mp &) = default;
    mp &operator=(const mp &) = default;
    mp(mp &&) = default;
    mp &operator=(mp &&) = default; 
};


}//namespace llama
