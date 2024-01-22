#pragma once
#include "exceptions.h"
#include <cstddef>
#include <compare>
namespace llama
{

/// 非空指针 T*。用它来表示已经判过空的指针。它没有空的状态，如果试图用空初始化它，会
/// 在运行时抛出 NullPointerException 异常。除此之外不会检查空，因此有更好的性能。
/// 没有默认构造函数。如果由于种种原因需要默认构造，请用np代替。
template <typename T>
class p
{
public:
	p(std::nullptr_t) = delete;

	/// 如果 U* 可以转为 T*，则 U* 也可以转为 p<T>
	p(T *ptr)
		: ptr(ptr)
	{
		if (!ptr)
			throw NullPointerException();
	}

	/// 如果 U* 可以转为 T*，则 p<U> 也可以转为 p<T>
	template <typename U>
	p(p<U> ptr)
		: ptr(ptr.data())
	{
		if (!ptr)
			throw NullPointerException();
	}

	operator T *() const { return ptr; }

	T &operator*() const { return *ptr; }
	T &deref() const { return *ptr; }

	T *operator->() const { return ptr; }
	T *data() const { return ptr; }

	T &operator[](ptrdiff_t offset) const { return ptr[offset]; }

	// p1 - p2
	ptrdiff_t operator-(p<T> p2) const { return ptr - p2.ptr; }
	// p + 1
	p<T> operator+(ptrdiff_t offset) const { return ptr + offset; }
	// 1 + p
	friend p<T> operator+(ptrdiff_t offset, p<T> p) { return p.ptr + offset; }
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

/// 可空指针 T*。
template <typename T>
class np
{
public:
	np()
		: ptr(nullptr)
	{}

	np(std::nullptr_t)
		: ptr(nullptr)
	{}

	/// 如果 U* 可以转为 T*，则 U* 也可以转为 np<T>
	np(T *ptr)
		: ptr(ptr)
	{}

	/// 如果 U* 可以转为 T*，则 np<U> 也可以转为 np<T>
	template <typename U>
	np(np<U> ptr)
		: ptr(ptr.data())
	{}

	/// 如果 U* 可以转为 T*，则 p<U> 可以转为 np<T>
	template <typename U>
	np(p<U> ptr)
		: ptr(ptr.data())
	{}

	explicit operator bool() const { return ptr; }
	operator T *() const { return ptr; }

	T &operator*() const
	{
		if (!ptr)
			throw NullPointerException();
		return *ptr;
	}
	T &deref() const
	{
		if (!ptr)
			throw NullPointerException();
		return *ptr;
	}

	T *operator->() const
	{
		if (!ptr)
			throw NullPointerException();
		return ptr;
	}

	p<T> unwrap() const
	{
		if (!ptr)
			throw NullPointerException();
		return ptr;
	}
	T *data() const { return ptr; }

	T &operator[](ptrdiff_t offset) const
	{
		if (!ptr)
			throw NullPointerException();
		return ptr[offset];
	}

	// np1 - np2
	ptrdiff_t operator-(np<T> p2) const { return ptr - p2.ptr; }
	// np + 1
	np<T> operator+(ptrdiff_t offset) const { return ptr + offset; }
	// 1 + np
	friend np<T> operator+(ptrdiff_t offset, np<T> p) { return p.ptr + offset; }
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

template <typename T>
class sp
{
public:
	sp() {}
};

}//namespace llama
