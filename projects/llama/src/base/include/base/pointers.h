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

class bypass_null_check
{
};

/// 非空指针 `T*` 。用它来表示已经判过空的指针。它没有空的状态，如果试图用空初始化它，会
/// 在运行时抛出 NullPointerException 异常。除此之外不会检查空，因此有更好的性能。
/// 没有默认构造函数。如果由于种种原因需要默认构造，请用np代替。
template <typename T> class p
{
public:
    /// 禁止用空指针构造。
    p(std::nullptr_t) = delete;

    /// 如果 `U*` 可以转为 `T*` ，则 `U*` 也可以转为 `p<T>`
    /// @exception 要求 `ptr` 非空。否则抛出异常。
    p(T *ptr) : ptr(ptr)
    {
        if (!ptr)
            throw NullPointerException();
    }

    /// 如果 `U*` 可以转为 `T*`，则 `p<U>` 也可以转为 `p<T>`
    /// @exception 要求 `ptr` 非空。否则抛出异常。
    template <typename U> p(p<U> ptr) : ptr(ptr.data())
    {
        if (!ptr)
            throw NullPointerException();
    }

    p(T *ptr, bypass_null_check) : ptr(ptr)
    {
    }

    template <typename U> p(p<U> ptr, bypass_null_check) : ptr(ptr.data())
    {
    }

    /// 可以隐式转换为普通指针
    operator T *() const
    {
        return ptr;
    }

    template <typename U> p<U> cast_static() const
    {
        return p<U>(static_cast<U *>(ptr), bypass_null_check{});
    }

    template <typename U> p<U> cast_reinterp() const
    {
        return p<U>(reinterpret_cast<U *>(ptr), bypass_null_check{});
    }

    template <typename U> p<U> cast_const() const
    {
        return p<U>(const_cast<U *>(ptr), bypass_null_check{});
    }

    /// 解引用。
    /// 如果 T=void ，无效。
    /// @exception 要求 `ptr` 非空。否则抛出异常。
    template <typename U = T> U &operator*() const
    {
        static_assert(!std::is_void<U>::value, "Cannot dereference a void pointer.");
        return *ptr;
    }

    /// 解引用。
    /// 如果 T=void ，无效。
    /// @exception 要求 `ptr` 非空。否则抛出异常。
    template <typename U = T> U &deref() const
    {
        static_assert(!std::is_void<U>::value, "Cannot dereference a void pointer.");
        return *ptr;
    }

    T *operator->() const
    {
        return ptr;
    }
    T *data() const
    {
        return ptr;
    }

    /// 访问偏移量
    /// 如果 T=void ，无效。
    /// @exception 要求 `ptr` 非空。否则抛出异常。
    template <typename U = T> U &operator[](ptrdiff_t offset) const
    {
        static_assert(!std::is_void<U>::value, "Cannot dereference a void pointer.");
        return ptr[offset];
    }

    // p1 - p2
    ptrdiff_t operator-(p<T> p2) const
    {
        return ptr - p2.ptr;
    }
    // p + 1
    p<T> operator+(ptrdiff_t offset) const
    {
        return ptr + offset;
    }
    // 1 + p
    friend p<T> operator+(ptrdiff_t offset, p<T> p)
    {
        return p.ptr + offset;
    }
    // p ++
    p<T> operator++(int)
    {
        p<T> old = *this;
        ++ptr;
        return old;
    }
    // ++ p
    p<T> &operator++()
    {
        ++ptr;
        return *this;
    }
    // p --
    p<T> operator--(int)
    {
        p<T> old = *this;
        --ptr;
        return old;
    }
    // -- p
    p<T> &operator--()
    {
        --ptr;
        return *this;
    }
    // p += 1
    p<T> &operator+=(ptrdiff_t offset)
    {
        ptr += offset;
        return *this;
    }
    // p -= 1
    p<T> &operator-=(ptrdiff_t offset)
    {
        ptr -= offset;
        return *this;
    }

private:
    T *ptr;
};

/// 可空指针 `T*` 。
template <typename T> class np
{
public:
    np() : ptr(nullptr)
    {
    }

    np(std::nullptr_t) : ptr(nullptr)
    {
    }

    /// 如果 `U*` 可以转为 `T*` ，则 `U*` 也可以转为 `np<T>`
    np(T *ptr) : ptr(ptr)
    {
    }

    /// 如果 `U*` 可以转为 `T*` ，则 `np<U>` 也可以转为 `np<T>`
    template <typename U> np(np<U> ptr) : ptr(ptr.data())
    {
    }

    /// 如果 `U*` 可以转为 `T*` ，则 `p<U>` 可以转为 `np<T>`
    template <typename U> np(p<U> ptr) : ptr(ptr.data())
    {
    }

    /// 可以在特定语境下转为 bool （用在 if / while 的条件里）
    explicit operator bool() const
    {
        return ptr;
    }

    /// 可以隐式转为普通指针。
    operator T *() const
    {
        return ptr;
    }

    template <typename U> np<U> cast_static() const
    {
        return np<U>(static_cast<U *>(ptr));
    }

    template <typename U> np<U> cast_reinterp() const
    {
        return np<U>(reinterpret_cast<U *>(ptr));
    }

    template <typename U> np<U> cast_const() const
    {
        return np<U>(const_cast<U *>(ptr));
    }

    /// 解引用。
    /// 如果 T=void ， 无效。
    /// @exception 要求 `ptr` 非空。否则抛出异常。
    template <typename U = T> U &operator*() const
    {
        static_assert(!std::is_void<U>::value, "Cannot dereference a void pointer.");
        if (!ptr)
            throw NullPointerException();
        return *ptr;
    }

    /// 解引用。
    /// 如果 T=void ， 无效。
    /// @exception 要求 `ptr` 非空。否则抛出异常。
    template <typename U = T> U &deref() const
    {
        static_assert(!std::is_void<U>::value, "Cannot dereference a void pointer.");
        if (!ptr)
            throw NullPointerException();
        return *ptr;
    }

    /// `->` 运算符
    /// @exception 要求 `ptr` 非空。否则抛出异常。
    T *operator->() const
    {
        if (!ptr)
            throw NullPointerException();
        return ptr;
    }

    /// 转为 `p<T>`
    /// @exception 要求 `ptr` 非空。否则抛出异常。
    p<T> unwrap() const
    {
        if (!ptr)
            throw NullPointerException();
        return ptr;
    }

    T *data() const
    {
        return ptr;
    }

    /// 访问偏移量
    /// 如果 T=void ，无效。
    /// @exception 要求 `ptr` 非空。否则抛出异常。
    template <typename U = T> U &operator[](ptrdiff_t offset) const
    {
        static_assert(!std::is_void<U>::value, "Cannot dereference a void pointer.");
        if (!ptr)
            throw NullPointerException();
        return ptr[offset];
    }

    // np1 - np2
    ptrdiff_t operator-(np<T> p2) const
    {
        return ptr - p2.ptr;
    }
    // np + 1
    np<T> operator+(ptrdiff_t offset) const
    {
        return ptr + offset;
    }
    // 1 + np
    friend np<T> operator+(ptrdiff_t offset, np<T> p)
    {
        return p.ptr + offset;
    }
    // np ++
    np<T> operator++(int)
    {
        np<T> old = *this;
        ++ptr;
        return old;
    }
    // ++ np
    np<T> &operator++()
    {
        ++ptr;
        return *this;
    }
    // np --
    np<T> operator--(int)
    {
        np<T> old = *this;
        --ptr;
        return old;
    }
    // -- np
    np<T> &operator--()
    {
        --ptr;
        return *this;
    }
    // np += 1
    np<T> &operator+=(ptrdiff_t offset)
    {
        ptr += offset;
        return *this;
    }
    // np -= 1
    np<T> &operator-=(ptrdiff_t offset)
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
