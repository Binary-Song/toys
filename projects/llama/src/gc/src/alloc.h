#include "base/exceptions.h"
#include "base/misc.h"
#include "base/pointers.h"
#include "iter.h"
#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <mutex>
#include <type_traits>

namespace llama
{
static constexpr size_t ALIGNMENT = 16;

class NeighbourIter;
class FreeChunkIter;
class Chunk;

class Links
{
public:
    Chunk *m_prev_freed = {};
    Chunk *m_next_freed = {};
};

inline NeighbourIter ToNeighbourIter(np<Chunk> chunk);
inline FreeChunkIter ToFreeChunkIter(np<Chunk> chunk);
inline p<Chunk> MergeNext(p<Chunk> chunk);
inline p<Chunk> MergePrev(p<Chunk> chunk);
inline p<Chunk> MergeConsecutive(p<Chunk> first, p<Chunk> second);
inline constexpr size_t AlignedSize(size_t);

class Chunk
{
public:
    /// 默认创建未被使用的Chunk。
    constexpr Chunk()
    {
        SetOccupied(false);
        SetSize(sizeof(Chunk));
        m_links.m_next_freed = nullptr;
        m_links.m_prev_freed = nullptr;
    }

    bool Occupied() const
    {
        return m_fields.m_occupied;
    }

    void SetOccupied(bool value)
    {
        m_fields.m_occupied = value;
    }

    bool HasNextNeighbour() const
    {
        return m_fields.m_has_next_neighbour;
    }

    void SetHasNextNeighbour(bool value)
    {
        m_fields.m_has_next_neighbour = value;
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

    np<Chunk> PrevNeighbour() const
    {
        if (!m_prev_size)
            return nullptr;
        return (Chunk *)((byte *)this - PrevSize());
    }

    np<Chunk> NextNeighbour() const
    {
        if (!m_fields.m_has_next_neighbour)
            return nullptr;
        return (Chunk *)((byte *)this + Size());
    }

    np<Chunk> PrevFreeChunk() const
    {
        if (!Occupied())
            return nullptr;
        return m_links.m_prev_freed;
    }

    np<Chunk> NextFreeChunk() const
    {
        if (!Occupied())
            return nullptr;
        return m_links.m_next_freed;
    }

    void SetPrevFreeChunk(np<Chunk> prev)
    {
        assert(!Occupied());
        m_links.m_prev_freed = prev;
    }

    void SetNextFreeChunk(np<Chunk> next)
    {
        assert(!Occupied());
        m_links.m_next_freed = next;
    }

    p<byte> GetData()
    {
        return m_data;
    }

    static p<Chunk> DataFieldToChunk(p<byte> data)
    {
        return p<Chunk>{(Chunk *)(data - offsetof(Chunk, m_data)), bypass_null_check{}};
    }

    size_t m_prev_size = {};
    struct Fields
    {
        size_t m_8xsize : sizeof(size_t) * 8 - 3, m_has_next_neighbour : 1, m_occupied : 1;
    } m_fields = {};
    static_assert(sizeof(ALIGNMENT) >= 8 && ALIGNMENT % 8 == 0, "ALIGNMENT must be a mulitple of 8");
    static_assert(sizeof(Fields) == sizeof(size_t), "Fields length must be the same as size_t.");
    union {
        Links m_links;  // active if m_occupied
        byte m_data[1]; // active if not m_occupied
    };
};

static_assert(std::is_standard_layout<Chunk>::value, "Chunk must be standard layout.");

static_assert(sizeof(Chunk) % ALIGNMENT == 0, "Chunk size must be padded to a multiple of ALIGNMENT");

class NeighbourIter final : public IBiIterator<NeighbourIter, Chunk>
{
public:
    explicit NeighbourIter(np<Chunk> chunk) : m_chunk(chunk)
    {
    }

private:
    virtual np<Chunk> Get_IIterator() const override
    {
        return m_chunk;
    }

    virtual void Prev_IIterator() override
    {
        m_chunk = m_chunk->PrevNeighbour();
    }

    virtual void Next_IIterator() override
    {
        m_chunk = m_chunk->NextNeighbour();
    }

    np<Chunk> m_chunk;
};

/// invalidated upon SetOccupied !!
class FreeChunkIter final : public IBiIterator<FreeChunkIter, Chunk>
{
public:
    explicit FreeChunkIter(np<Chunk> chunk) : m_chunk(chunk->Occupied() ? nullptr : chunk)
    {
    }

    void Erase()
    {
        p<Chunk> chunk = m_chunk.unwrap();
        assert(chunk->Occupied() == false);
        if (const auto prev = chunk->PrevFreeChunk())
        {
            prev->SetNextFreeChunk(chunk->NextFreeChunk());
        }
        if (const auto next = chunk->NextFreeChunk())
        {
            next->SetPrevFreeChunk(chunk->PrevFreeChunk());
        }
    }

private:
    virtual np<Chunk> Get_IIterator() const override
    {
        return m_chunk;
    }

    virtual void Prev_IIterator() override
    {
        m_chunk = m_chunk->PrevFreeChunk();
    }

    virtual void Next_IIterator() override
    {
        m_chunk = m_chunk->NextFreeChunk();
    }

    np<Chunk> m_chunk;
};

class Arena
{
public:
    explicit Arena(size_t size) : m_begin((byte *)malloc(size)), m_end((byte *)m_begin + size), m_current(m_begin)
    {
        assert(size_t(m_begin.as_raw()) % ALIGNMENT == 0);
        if (!m_begin)
        {
            throw Exception("malloc failed");
        }
    }

    Arena(const Arena &) = delete;
    Arena &operator=(const Arena &) = delete;

    ~Arena()
    {
        free(m_begin.as_raw());
    }

    p<Chunk> RequestChunk()
    {
        auto chunk = Request(sizeof(Chunk)).unwrap().reinterpret_as<Chunk>();
        new (chunk) Chunk{};
        chunk->SetHasNextNeighbour(false);
        return chunk;
    }

    void ReturnChunk(p<Chunk> lastChunk)
    {
        p<byte> lastOne = lastChunk.reinterpret_as<byte>();
        assert(lastChunk->Size() + lastOne == m_current);
        lastChunk->SetHasNextNeighbour(true);
        assert(m_current.reinterpret_as<Chunk>()->HasNextNeighbour() == false);
        m_current = lastOne;
    }

private:
    np<byte> Request(size_t size)
    {
        if (m_current + size > m_end)
        {
            return nullptr;
        }
        m_current += size;
        return m_current;
    }

    bool Return(p<byte> ptr)
    {
        if (!(ptr >= m_begin && ptr <= m_current))
            return false;
        m_current = ptr;
        return true;
    }

private:
    p<byte> m_begin;
    p<byte> m_end;
    p<byte> m_current;
};

class FreeList
{
public:
    explicit FreeList(p<Arena> arena) : m_arena(arena), m_head(m_arena->RequestChunk())
    {
    }

    FreeChunkIter InsertNew()
    {
        auto chunk = m_arena->RequestChunk();
        return Insert(chunk);
    }

    FreeChunkIter Insert(p<Chunk> chunk)
    {
        assert(chunk->Occupied() == false);
        if (const auto next = m_head->NextFreeChunk())
        {
            m_head->SetNextFreeChunk(chunk);
            chunk->SetPrevFreeChunk(m_head);

            next->SetPrevFreeChunk(chunk);
            chunk->SetNextFreeChunk(next);
        }
        else
        {
            m_head->SetNextFreeChunk(chunk);
            chunk->SetPrevFreeChunk(m_head);
        }
        return FreeChunkIter{chunk};
    }

    np<Chunk> Pop()
    {
        if (Chunk *chunk = m_head->NextFreeChunk())
        {
            return chunk;
        }
        return nullptr;
    }

    FreeChunkIter begin()
    {
        return FreeChunkIter{m_head->NextFreeChunk()};
    }

    FreeChunkIter end()
    {
        return FreeChunkIter{nullptr};
    }

private:
    p<Arena> m_arena;
    p<Chunk> m_head;
};

class Allocator
{
public:
    static constexpr size_t NUM_SMALL_BINS = 64;
    static constexpr size_t NUM_LARGE_BINS = 64;
    static constexpr size_t MIN_LARGE_BIN_SIZE = ALIGNMENT * (NUM_SMALL_BINS + 1);

    np<byte> Malloc(size_t size)
    {
        size = AlignedSize(size);

        // go through small bin
        if (FreeList *bin = GetBin(size))
        {
            if (Chunk *chunk = bin->Pop())
            {
                return MarkAllocated(chunk);
            }
        }

        // go through unsorted bin
        for (Chunk &unsorted_item : m_unsorted)
        {
            if (unsorted_item.Size() == size)
            {
                ToFreeChunkIter(&unsorted_item).Erase();
                return MarkAllocated(&unsorted_item);
            }
            else
            {
            }
        }
    }

    void Free(np<byte> n_data)
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        if (!n_data)
            return;
        p<byte> data = n_data.unwrap();
        p<Chunk> chunk = Chunk::DataFieldToChunk(data);
        chunk = MergePrev(chunk);
        chunk = MergeNext(chunk);
        if (!chunk->HasNextNeighbour()) // last one in arena
        {
            m_arena.ReturnChunk(chunk);
            return;
        }
        m_unsorted.Insert(chunk);
    }

private:
    static constexpr size_t GetLargeBinIndex(size_t size)
    {
        return (size_t)log2(
            double(size - MIN_LARGE_BIN_SIZE) / 16 + 1);
    }

    np<FreeList> GetBin(size_t size)
    {
        // small bin
        if (size < MIN_LARGE_BIN_SIZE)
        {
            size_t idx = AlignedSize(size) / ALIGNMENT - 1;
            if (idx >= m_small.size())
                return nullptr; 
            return &m_small[idx];
        }
        // large bin
        else
        {
            size_t idx = GetLargeBinIndex(size);
            if (idx >= m_large.size())
                return nullptr;
            return &m_large[idx];
        }
    }

    p<byte> MarkAllocated(p<Chunk> chunk)
    {
        chunk->SetOccupied(true);
        return chunk->GetData();
    }

private:
    std::mutex m_mtx;
    Arena m_arena;
    FreeList m_unsorted;
    std::array<FreeList, NUM_SMALL_BINS> m_small;
    std::array<FreeList, NUM_LARGE_BINS> m_large;
    // static thread_local std::array<FreeList, NUM_TCACHE_BINS> m_tcache;
};

NeighbourIter ToNeighbourIter(np<Chunk> chunk)
{
    return NeighbourIter{chunk};
}

FreeChunkIter ToFreeChunkIter(np<Chunk> chunk)
{
    return FreeChunkIter{chunk};
}

p<Chunk> MergePrev(p<Chunk> chunk)
{
    if (Chunk *nb = chunk->PrevNeighbour())
    {
        return MergeConsecutive(nb, chunk);
    }
    return chunk;
}

p<Chunk> MergeNext(p<Chunk> chunk)
{
    if (Chunk *nb = chunk->NextNeighbour())
    {
        return MergeConsecutive(chunk, nb);
    }
    return chunk;
}

p<Chunk> MergeConsecutive(p<Chunk> first, p<Chunk> second)
{
    const bool first_free = !first->Occupied();
    const bool second_free = !second->Occupied();

    assert(first_free || second_free);

    if (first_free)
    {
        if (second_free)
        {
            ToFreeChunkIter(first).Erase();
            ToFreeChunkIter(second).Erase();
            first->SetSize(first->Size() + second->Size());
            first->SetHasNextNeighbour(second->HasNextNeighbour());
        }
        return first;
    }
    // first not free
    return second;
}

constexpr size_t AlignedSize(size_t size)
{
    if (!size)
        size = 1;
    const size_t rem = (size - 1) % ALIGNMENT;
    return (size - 1) - rem + ALIGNMENT;
}

} // namespace llama