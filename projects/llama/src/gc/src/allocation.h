#pragma once
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>

namespace llama
{
namespace gc
{

class Chunk
{
public:
    size_t Size() const
    {
        return m_size_and_flags & ~0x7;
    }

    bool IsPrevInUse() const
    {
        return m_size_and_flags & 0x1;
    }

    static Chunk *DataPtrToChunkPtr(uint8_t *ptr)
    {
        return (Chunk *)(ptr - OVERHEAD_SIZE);
    }

    Chunk *PrevFreeChunkOrNull()
    {
        if (!IsPrevInUse())
        {
            size_t prev_size = m_prev_size_if_free;
            Chunk *prev = (Chunk *)((uint8_t *)this - prev_size);
            return prev;
        }
        return nullptr;
    }

    void Update(bool prev_in_use, size_t prev_size, size_t size)
    {
        assert(size % 16 == 0);
        if (!prev_in_use)
            m_prev_size_if_free = prev_size;
        m_size_and_flags = size + (size_t)prev_in_use;
    }

    void Update(size_t size)
    {
        assert(size % 16 == 0);
        m_size_and_flags = size + (size_t)IsPrevInUse();
    }

    struct Links
    {
        /// bin中的上一个chunk
        Chunk *m_back;
        /// bin中的下一个chunk （和prev区分开）
        Chunk *m_forward;
    };

public:
    // 如果前一个chunk未分配，则存放其大小。否则为用户数据。
    size_t m_prev_size_if_free;
    // 后三位是flags。如果将后三位设置为0，则为Chunk的size（包括overhead）。
    // 后三位：A M P
    // P - 前一个chunk是不是分配了。0=未分配，1=已分配
    size_t m_size_and_flags;

    union {
        uint8_t m_data[1];
        Links m_links;
    } m_union;

    static constexpr size_t OVERHEAD_SIZE = sizeof(m_prev_size_if_free) + sizeof(m_size_and_flags);
};



class Heap
{
public:
    static constexpr size_t DEFAULT_SIZE = 1073741824;
    static constexpr size_t ALIGNMENT = 16;
    static constexpr size_t SMALLBINS = 62;

    static Heap Create(size_t heap_size = DEFAULT_SIZE)
    {
        assert(heap_size % ALIGNMENT == 0);

        Heap heap;
        heap.m_begin = ((uint8_t *)malloc(heap_size));
        heap.m_end = heap.m_begin + heap_size;
        heap.m_top_chunk = (Chunk *)(heap.m_begin);

        assert(heap.m_begin);

        // init top chunk
        heap.m_top_chunk->m_prev_size_if_free = 0;            // 第一个chunk P=1
        heap.m_top_chunk->m_size_and_flags = heap_size + 0x1; // P=1
        heap.m_top_chunk->m_union.m_links.m_back = heap.m_top_chunk;
        heap.m_top_chunk->m_union.m_links.m_forward = heap.m_top_chunk;

        return heap;
    }

    constexpr static size_t AlignedSize(size_t size)
    {
        return ((size + 15) / 16) * 16;
    }

    uint8_t *Alloc(size_t alloc_size)
    {
        assert(alloc_size);
        alloc_size = AlignedSize(alloc_size);
        size_t small_bin_idx = alloc_size / 16 - 1;
        assert(small_bin_idx < SMALLBINS);
        Chunk *small_bin = m_small_bins[small_bin_idx];
        if (small_bin)
        {
            // if small bin is not empty, get first one and return
            Chunk *chunk = PopFront(small_bin);
            return chunk->m_union.m_data;
        }
        else
        {
            // search the unsorted bin
            if (m_unsorted_bin)
            {
                Chunk *bin_item = m_unsorted_bin;
                if (bin_item->Size() == alloc_size)
                {
                    if (Chunk *next = NextChunkOrNull(bin_item))
                    {
                        next->m_size_and_flags |= 0x1; // next.P = 1
                    }
                    // update fw and bk
                    
                    return bin_item;
                }

                bin_item = m_unsorted_bin->m_union.m_links.m_forward;
            }
            else
            {
                // no? slice a new chunk from top chunk
                Chunk *const chunk = NewChunk(alloc_size);
                if (!chunk)
                    return nullptr;
                return chunk->m_union.m_data;
            }
        }
    }

    void Free(uint8_t *data)
    {
        if (!data)
            return;

        Chunk *const chunk = Chunk::DataPtrToChunkPtr(data);
        Chunk *merged_chunk = chunk;

        bool put_into_unsorted = true;
        // 1. merge with neighbors
        if (Chunk *prev_chunk = chunk->PrevFreeChunkOrNull())
        {
            // if prev chunk is free, merge
            merged_chunk = prev_chunk;
            const size_t merged_size = prev_chunk->Size() + chunk->Size();
            prev_chunk->Update(merged_size);
            if (Chunk *next_chunk = NextChunkOrNull(chunk))
            {
                next_chunk->Update(false, merged_size, next_chunk->Size());
            }
        }
        if (Chunk *next_chunk = NextFreeChunkOrNull(chunk))
        {
            // if next chunk is free, merge
            if (next_chunk == m_top_chunk)
            {
                // update top chunk pointer
                m_top_chunk = merged_chunk;
                put_into_unsorted = false;
            }
            const size_t merged_size = merged_chunk->Size() + next_chunk->Size();
            merged_chunk->Update(merged_size);
            if (Chunk *second_next_chunk = NextChunkOrNull(next_chunk))
            {
                second_next_chunk->Update(false, merged_size, second_next_chunk->Size());
            }
        }

        // 2. insert into unsorted bin
        if (put_into_unsorted)
        {
            m_unsorted_bin = InsertFront(m_unsorted_bin, merged_chunk);
        }
    }

    Chunk *NewChunk(size_t aligned_size)
    {
        assert(aligned_size % 16 == 0);
        Chunk *const new_chunk = m_top_chunk;
        Chunk *const old_top_chunk = m_top_chunk;
        Chunk *const new_top_chunk = m_top_chunk + aligned_size;
        Chunk *const new_top_chunk_end = new_top_chunk + sizeof(Chunk);
        if ((uint8_t *)new_top_chunk_end > m_end)
        {
            return nullptr;
        }

        // note: if new flags are added, remember to update them here.

        // compute values
        const bool old_chunk_prev_in_use = old_top_chunk->IsPrevInUse();
        const size_t new_chunk_size = Chunk::OVERHEAD_SIZE + aligned_size;
        const size_t old_top_chunk_size = old_top_chunk->Size();
        const size_t new_top_chunk_size = old_top_chunk_size - aligned_size;

        // assign them (using computed constants only)
        new_chunk->Update(new_chunk_size);
        new_top_chunk->Update(true, new_chunk_size, new_top_chunk_size);
        return new_chunk;
    }

    Chunk *NextFreeChunkOrNull(Chunk *chunk)
    {
        Chunk *next_chunk = (Chunk *)((uint8_t *)chunk + chunk->Size());
        if (next_chunk == m_top_chunk)
        {
            return nullptr;
        }
        Chunk *second_next_chunk = (Chunk *)((uint8_t *)next_chunk + next_chunk->Size());
        if (second_next_chunk->IsPrevInUse())
        {
            return nullptr;
        }
        return next_chunk;
    }

    Chunk *NextChunkOrNull(Chunk *chunk)
    {
        if (chunk == m_top_chunk)
            return nullptr;
        Chunk *next_chunk = (Chunk *)((uint8_t *)chunk + chunk->Size());
        return next_chunk;
    }

    static Chunk *InsertFront(Chunk *bin_head, Chunk *inserted)
    {
        if (bin_head)
        {
            bin_head->m_union.m_links.m_back = inserted;
        }
        inserted->m_union.m_links.m_forward = bin_head;
        inserted->m_union.m_links.m_back = nullptr;
        // update bin head
        return inserted;
    }

    static Chunk *PopFront(Chunk *&bin_head)
    {
        if (!bin_head)
        {
            return nullptr;
        }

        Chunk *const head = bin_head;
        if (Chunk *next = bin_head->m_union.m_links.m_forward)
        {
            next->m_union.m_links.m_back = nullptr;
            bin_head = next;
        }
        else
        {
            bin_head = nullptr;
        }
        return head;
    }

    uint8_t *m_begin{};
    uint8_t *m_end{};
    Chunk *m_top_chunk{};
    Chunk *m_small_bins[SMALLBINS]{};
    Chunk *m_unsorted_bin{};
};

} // namespace gc
} // namespace llama