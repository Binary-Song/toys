#include "base/exceptions.h"
#include "base/misc.h"
#include "base/pointers.h"
#include "iter.h"
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <unordered_map>
#include <mutex>
namespace llama
{
static constexpr size_t ALIGNMENT = 16;

class Chunk;
class Links
{
public:
    Chunk *m_prev_freed = {};
    Chunk *m_next_freed = {};
};

inline class NeighbourIter GetNeighbourIter(np<Chunk> chunk);
inline class FreeChunkIter GetBinIter(np<Chunk> chunk);

class Chunk
{
public:
    /// 默认创建未被使用的Chunk。
    Chunk()
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

private:
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
    explicit Arena(size_t size) : m_begin((byte *)malloc(size)), m_end((byte *)m_begin + size)
    {
        if (!m_begin)
        {
            throw Exception("malloc failed");
        }
    }

    p<Chunk> RequestChunk()
    {
        auto chunk = Request(sizeof(Chunk)).unwrap();
        new (chunk) Chunk{};
        return chunk;
    }

    bool TryReturnChunk(p<Chunk> lastChunk)
    {
        byte* chunk_begin = (byte*)lastChunk.as_raw();
        if (lastChunk->Size() + chunk_begin == m_current)
        {
            m_current = chunk_begin;
            return true;
        }
        return false;
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
    byte *m_begin;
    byte *m_current;
    byte *m_end;
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

    void Erase(p<Chunk> chunk)
    {
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
    p<Arena> m_arena;
    p<Chunk> m_head;
};

/// 多个线程共享的内存池
class SharedAllocator
{
public:
    np<byte> Allocate(size_t size)
    {
        if(m_sorted)
    }

    void Free()
    {

    }

private:
    std::mutex m_mtx;
    Arena m_arena;
    FreeList m_unsorted;
    std::unordered_map<size_t, FreeList> m_sorted;
};

/// 仅被单线程访问的内存池
class ExclusiveAllocator
{
public:
};

NeighbourIter GetNeighbourIter(np<Chunk> chunk)
{
    return NeighbourIter{chunk};
}

FreeChunkIter GetFreeChunkIter(np<Chunk> chunk)
{
    return FreeChunkIter{chunk};
}

} // namespace llama