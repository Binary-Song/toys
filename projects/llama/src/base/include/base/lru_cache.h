/*

Copyright (c) 2014, lamerman
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of lamerman nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

/*
 * File:   lrucache.hpp
 * Author: Alexander Ponomarev
 *
 * Created on June 20, 2013, 5:09 PM
 */

#ifndef _LRUCACHE_HPP_INCLUDED_
#define _LRUCACHE_HPP_INCLUDED_

#include "exceptions.h"
#include <cstddef>
#include <list>
#include <map>

namespace llama
{

/// @brief 一个简单的 LRU 缓存。
/// @details LRU 缓存是一个类似 map 的数据结构，可以存取键值对。
/// 用 lru_cache::put 向缓存中插入键值对时，如果超过缓存的最大容量，则
/// 缓存会将最久没访问的键值对丢弃。重写虚函数 lru_cache::overflow 来自定义丢弃行为。
/// 用 lru_cache::get 访问键值对。访问需要 key 。如果缓存中不包含这个 key ，
/// 则虚函数 lru_cache::miss 被调用，默认行为是抛出异常。
template <typename key_t, typename value_t> class lru_cache
{
public:
    typedef typename std::pair<key_t, value_t> key_value_pair_t;
    typedef typename std::list<key_value_pair_t>::iterator list_iterator_t;

    /// 构造。
    /// @param max_size 缓存最多能装下几个键值对。
    lru_cache(size_t max_size) : _max_size(max_size)
    {
    }

    /// 向缓存中插入键值对。
    void put(const key_t &key, const value_t &value)
    {
        auto it = _cache_items_map.find(key);
        _cache_items_list.push_front(key_value_pair_t(key, value));
        if (it != _cache_items_map.end())
        {
            _cache_items_list.erase(it->second);
            _cache_items_map.erase(it);
        }
        _cache_items_map[key] = _cache_items_list.begin();

        if (_cache_items_map.size() > _max_size)
        {
            auto last = _cache_items_list.end();
            last--;
            overflow(last->first, last->second);
            _cache_items_map.erase(last->first);
            _cache_items_list.pop_back();
        }
    }

    /// 给定 key ，获得 value 。
    value_t get(const key_t &key)
    {
        auto it = _cache_items_map.find(key);
        if (it == _cache_items_map.end())
        {
            value_t val = miss(key);
            put(key, val);
            return val;
        }
        else
        {
            _cache_items_list.splice(_cache_items_list.begin(), _cache_items_list, it->second);
            return it->second->second;
        }
    }

    /// key 是否存在。
    bool exists(const key_t &key) const
    {
        return _cache_items_map.find(key) != _cache_items_map.end();
    }

    /// 当前缓存中键值对数量。
    size_t size() const
    {
        return _cache_items_map.size();
    }

    /// 缓存最多能装下几个键值对。
    size_t max_size() const
    {
        return _max_size;
    }

private:
    /// 重写虚函数 lru_cache::overflow 来自定义丢弃行为。默认无操作。
    virtual void overflow(const key_t &key, const value_t &value)
    {
    }

    /// 重写虚函数 lru_cache::miss 来自定义找不到 key 时的行为。
    /// 如果能从其他地方找到这个 key 的 value ，就返回它。
    /// 默认抛 Exception 。
    virtual value_t miss(const key_t &key)
    {
        throw llama::Exception{"lru_cache: Key not found."};
    }

private:
    std::list<key_value_pair_t> _cache_items_list;
    std::map<key_t, list_iterator_t> _cache_items_map;
    size_t _max_size;
};

} // namespace llama

#endif /* _LRUCACHE_HPP_INCLUDED_ */