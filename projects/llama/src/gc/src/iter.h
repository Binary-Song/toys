
#include "base/misc.h"
#include "base/pointers.h"
#include <cassert>
#include <cstddef>
#include <unordered_map>

namespace llama
{

/// 迭代器。
/// 派生类可以加final。
template <typename Self, typename E> class IIterator
{
public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = E;
    using pointer = E *;
    using reference = E &;

public:
    virtual ~IIterator() = default;

public:
    E &operator*() const
    {
        if (np<E> ptr = Deref_IIterator())
        {
            return *ptr;
        }
        throw NullPointerException();
    }

    E *operator->() const
    {
        return Deref_IIterator();
    }

    Self &operator++()
    {
        Next_IIterator();
        return *this;
    }

    bool operator==(const Self &other) const
    {
        return other.Deref_IIterator() == this->Deref_IIterator();
    }

    bool operator!=(const Self &other) const
    {
        return other.Deref_IIterator() != this->Deref_IIterator();
    }

private:
    virtual np<E> Get_IIterator() const = 0;
    virtual void Next_IIterator() = 0;
};

/// 双向迭代器。
template <typename Self, typename E> class IBiIterator : public IIterator<Self, E>
{
public:
    Self &operator--()
    {
        Prev_IIterator();
        return *this;
    }

private:
    virtual np<E> Get_IIterator() const = 0;
    virtual void Prev_IIterator() = 0;
};

} // namespace llama
