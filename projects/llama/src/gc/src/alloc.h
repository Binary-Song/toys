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
    virtual np<E> Deref_IIterator() const = 0;
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
    virtual np<E> Deref_IIterator() const = 0;
    virtual void Prev_IIterator() = 0;
};

static constexpr size_t ALIGNMENT = 16;

class Chunk;
class Links
{
public:
    Chunk *m_prev_freed = {};
    Chunk *m_next_freed = {};
};

class Chunk
{
public:
    bool Occupied() const
    {
        return m_fields.m_occupied;
    }

    void SetOccupied(bool value)
    {
        m_fields.m_occupied = value;
    }

    size_t Size() const
    {
        return m_fields.m_8xsize / 8;
    }

    void SetSize(size_t aligned_val)
    {
        assert(aligned_val % ALIGNMENT == 0);
        m_fields.m_8xsize = aligned_val * 8;
    }

    size_t PrevSize() const
    {
        return m_prev_size;
    }

    void SetPrevSize(size_t aligned_val)
    {
        assert(aligned_val % ALIGNMENT == 0);
        m_prev_size = aligned_val;
    }

private:
    size_t m_prev_size = {};
    struct Fields
    {
        size_t m_8xsize : sizeof(size_t) * 8 - 3, m_has_next_neighbor : 1, m_occupied : 1;
    } m_fields;
    static_assert(sizeof(ALIGNMENT) >= 8 && ALIGNMENT % 8 == 0, "ALIGNMENT must be a mulitple of 8");
    static_assert(sizeof(Fields) == sizeof(size_t), "Fields length must be the same as size_t.");
    union {
        Links m_links;  // active if m_occupied
        byte m_data[1]; // active if not m_occupied
    };
};

class Bin
{
public:
private:
};

class Allocator
{
};

} // namespace llama