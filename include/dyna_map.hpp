#ifndef DYNA_MAP_HPP
#define DYNA_MAP_HPP

#include <stdint.h>
#include <functional>
#include <utility>

#include "dyna_subtable.hpp"
#include "dyna_thread.hpp"

namespace dyna
{

  template <typename K, typename V, typename H = std::hash<K>, thread T = thread::safe>
  class map
  {
    static const size_t DEFAULT_MAP_SIZE = 256;

    subtable<K, V, H, T> *subtables;

    // might not need these inside tables
    iterator<K, V, H> head;
    iterator<K, V, H> tail;

    size_t capacity;
    size_t occupied;

    inline size_t hash_idx(size_t &hash_val) { return hash_val % capacity; }

    std::pair<bool, iterator<K, V, H>> lookup(K &key)
    {
      size_t hash_val = hash_func<K, H>(key);
      size_t i = hash_idx(hash_val);
      subtable<K, V, H, T> *table = subtables[i];

      return table->lookup(hash_val);
    }

  public:
    map()
    {
      subtables = new subtable<K, V, H, T> [DEFAULT_MAP_SIZE];
      capacity = DEFAULT_MAP_SIZE;
      head = nullptr;
      tail = nullptr;
      occupied = 0;
    }

    map(size_t custom_capacity)
    {
      subtables = new subtable<K, V, H, T> [custom_capacity];
      capacity = custom_capacity;
      head = nullptr;
      tail = nullptr;
      occupied = 0;
    }

    ~map()
    {
      delete[] subtables;
      head = nullptr;
      tail = nullptr;
    }

    inline iterator<K, V, H> begin() { return head; }
    inline iterator<K, V, H> end() { return tail->next; }

    iterator<K, V, H> find(K key)
    {
      std::pair<bool, iterator<K, V, H>> result = lookup(key);
      if (!result.first)
        return end();
      
      return result.second;
    } 

    // getter
    V &operator[](K key)
    {
      size_t hash_val = hash_func<K, H>(key);
      size_t i = hash_idx(hash_val);
      subtable<K, V, H, T> &table = subtables[i];
      return table.get(hash_val, key);
    }

    // setter
    void set(K key, V value)
    {
      node<K, V, H> *new_node = new node<K, V, H>(key, value);
      size_t i = hash_idx(new_node->hash);
      subtable<K, V, H, T> &table = subtables[i];
      table.set(new_node);
    }

    bool erase(K key)
    {
      size_t hash_val = hash_func<K, H>(key);
      size_t i = hash_idx(hash_val);
      subtable<K, V, H, T> &table = subtables[i];
      return table.erase(hash_val);
    }

    bool exists(K key)
    {
      std::pair<bool, iterator<K, V, H>> result = lookup(key);
      return result.first;
    }

    inline bool empty() { return !occupied; }
    inline size_t size() { return occupied; }
  };

}

#endif