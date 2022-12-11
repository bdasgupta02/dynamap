#ifndef DYNA_MAP_HPP
#define DYNA_MAP_HPP

#include <stdint.h>
#include <functional>
#include <utility>
#include <shared_mutex>
#include <mutex>

#include "dyna_subtable.hpp"
#include "dyna_thread.hpp"

namespace dyna
{

  template <typename K, typename V, typename H = std::hash<K>, thread T = thread::safe>
  class map
  {
    using iterator = iterator<K, V, H>;

    static const size_t DEFAULT_MAP_SIZE = 256;

    subtable<K, V, H, T> *subtables;
    iterator head;
    iterator tail;

    mutable std::shared_mutex *sub_mutexes;

    size_t capacity;
    size_t occupied;

    inline size_t hash_idx(size_t &hash_val) { return hash_val % capacity; }

    std::pair<bool, iterator> lookup(K &key)
    {
      size_t hash_val = hash_func<K, H>(key);
      size_t i = hash_idx(hash_val);
      subtable<K, V, H, T> *table = subtables[i];
      return table->lookup(hash_val);
    }

  public:
    map()
    {
      subtables = new subtable<K, V, H, T>[DEFAULT_MAP_SIZE];
      sub_mutexes = new std::shared_mutex[DEFAULT_MAP_SIZE];
      capacity = DEFAULT_MAP_SIZE;
      head = nullptr;
      tail = nullptr;
      occupied = 0;
    }

    map(size_t custom_capacity)
    {
      subtables = new subtable<K, V, H, T>[custom_capacity];
      sub_mutexes = new std::shared_mutex[custom_capacity];
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

    inline iterator begin() { return head; }
    inline iterator end() { return tail->next; }

    iterator find(K key)
    {
      std::pair<bool, iterator> result = lookup(key);
      if (!result.first)
        return end();

      return result.second;
    }

    // getter
    V &operator[](K key)
    {
      size_t hash_val = hash_func<K, H>(key);
      size_t i = hash_idx(hash_val);
      std::shared_lock lock{sub_mutexes[i], std::defer_lock};
      if (T == thread::safe)
        lock.lock();

      subtable<K, V, H, T> &table = subtables[i];
      return table.get(hash_val, key);
    }

    // setter
    void set(K key, V value)
    {
      node<K, V, H> *new_node = new node<K, V, H>(key, value);
      size_t i = hash_idx(*new_node->hash);
      std::unique_lock lock{sub_mutexes[i], std::defer_lock};
      if (T == thread::safe)
        lock.lock();

      if (!head)
      {
        head = new_node;
        tail = new_node;
      }
      else
      {
        tail->next = new_node;
        new_node->prev = tail;
        tail = tail->next;
      }

      subtable<K, V, H, T> &table = subtables[i];
      table.set(new_node);
    }

    bool erase(K key)
    {
      size_t hash_val = hash_func<K, H>(key);
      size_t i = hash_idx(hash_val);
      std::unique_lock lock{sub_mutexes[i], std::defer_lock};
      if (T == thread::safe)
        lock.lock();

      subtable<K, V, H, T> &table = subtables[i];

      std::pair<bool, iterator> sub_erase = table->erase(hash_val);
      if (!sub_erase.first)
        return false;

      if (*head->hash == *sub_erase.second->hash)
        head = sub_erase.second->next;

      if (*tail->hash == *sub_erase.second->hash)
        tail = sub_erase.second->prev;

      if (sub_erase.second->next)
        sub_erase.second->next->prev = sub_erase->prev;

      if (sub_erase.second->prev)
        sub_erase.second->prev->next = sub_erase->next;

      return true;
    }

    bool exists(K key)
    {
      std::pair<bool, iterator> result = lookup(key);
      return result.first;
    }

    inline bool empty() { return !occupied; }
    inline size_t size() { return occupied; }
    inline size_t max_size() { return capacity; }

    size_t max_size_deep() {
      size_t result = 0;
      for (int i = 0; i < capacity; i++)
        result += subtables[i].max_size();

      return result;
    }
  };

}

#endif