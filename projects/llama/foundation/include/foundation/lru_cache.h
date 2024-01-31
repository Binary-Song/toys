#pragma once
#include <list>
#include <unordered_map>

template <typename Key, typename Val> class LruCache
{
  public:
    explicit LruCache(size_t cap) : m_cap(cap)
    {
    }

  private:
    virtual void OverflowAsLruCache()
    {
    }

    virtual void UnderflowAsLruCache()
    {
    }

  public:
    void Put(const Key &key, const Val &value)
    {
        ListIter it = m_map.find(key);
        m_list.push_front({key, value});
        if (it != m_list.end())
        {
            m_list.erase(it->second);
            m_map.erase(it);
        }
        m_map[key] = m_list.begin();

        if (m_map.size() > m_cap)
        {
            auto last = m_list.end();
            last--;
            _cache_items_map.erase(last->first);
            _cache_items_list.pop_back();
        }
    }

    const value_t &get(const key_t &key)
    {
        auto it = _cache_items_map.find(key);
        if (it == _cache_items_map.end())
        {
            throw std::range_error("There is no such key in cache");
        }
        else
        {
            _cache_items_list.splice(_cache_items_list.begin(), _cache_items_list, it->second);
            return it->second->second;
        }
    }

    bool exists(const key_t &key) const
    {
        return _cache_items_map.find(key) != _cache_items_map.end();
    }

    size_t size() const
    {
        return _cache_items_map.size();
    }

  private:
    using ListIter = typename std::list<std::pair<Key, Val>>::iterator;
    size_t m_cap;
    std::list<std::pair<Key, Val>> m_list;
    std::unordered_map<Key, ListIter> m_map;
};