#ifndef DYNA_TABLE_HPP
#define DYNA_TABLE_HPP

#include <stdint.h>
#include <utility>
#include <shared_mutex>
#include <mutex>
#include <condition_variable>

#include <iostream>

#include "dyna_thread.hpp"
#include "dyna_node.hpp"

namespace dyna
{

  template <typename K, typename V, typename H, thread T>
  class subtable
  {
    static const uint16_t LOAD_FACTOR = 80;
    static const size_t INIT_SIZE = 8;

    node<K, V, H> **nodes;
    iterator<K, V, H> head;
    iterator<K, V, H> tail;

    mutable std::mutex ext_mutex;
    mutable std::condition_variable ext_cv;
    bool ext_done = true;

    mutable std::shared_mutex *mutexes;

    size_t capacity;
    size_t occupied;

    inline size_t hash_idx(size_t &hash_val) { return hash_val % capacity; }

    void extend()
    {
      std::unique_lock lock{ext_mutex, std::defer_lock};
      if (T == thread::safe)
      {
        lock.lock();
        ext_done = false;
      }

      delete[] nodes;
      delete[] mutexes;

      capacity *= 2;
      occupied = 0;

      nodes = new iterator<K, V, H> [capacity];
      mutexes = new std::shared_mutex[capacity];

      iterator<K, V, H> tmp = head;
      head = nullptr;
      tail = nullptr;

      while (tmp)
      {
        tmp->next_bucket = nullptr;
        set(tmp, false);
        tmp = tmp->next;
      }
      
      if (T == thread::safe)
      {
        ext_done = true;
        lock.unlock();
        ext_cv.notify_all();
      }
    }

    void ext_wait()
    {
      std::unique_lock lock(ext_mutex);
      ext_cv.wait(lock, [this]{ return ext_done; });
    }

  public:

    subtable()
    {
      nodes = new iterator<K, V, H>[INIT_SIZE]();
      mutexes = new std::shared_mutex[INIT_SIZE];
      head = nullptr;
      tail = nullptr;
      capacity = INIT_SIZE;
      occupied = 0;
    }

    ~subtable()
    {
      delete[] nodes;
      head = nullptr;
      tail = nullptr;
    }

    std::pair<bool, iterator<K, V, H>> lookup(size_t &hash_val)
    {
      size_t i = hash_idx(hash_val);

      if (T == thread::safe)
        std::shared_lock lock(mutexes[i]);

      iterator<K, V, H> node = nodes[i];

      while (node)
      {
        if (node->hash == hash_val)
          return std::make_pair(true, node);

        node = node->next_bucket;
      }

      return std::make_pair(false, nullptr);
    }

    V &get(size_t &hash_val, K &key)
    {
      ext_wait();

      std::pair<bool, iterator<K, V, H>> result = lookup(hash_val);
      if (result.first)
        return *result.second->second;
      else
      {
        V def_val;
        node<K, V, H> *new_node = new node<K, V, H>(key, def_val, hash_val);
        set(new_node);
        return get(hash_val, key);
      }
    }

    void set(node<K, V, H> *new_node, bool wait = true)
    {
      if (T == thread::safe && wait)
        ext_wait();

      std::pair<bool, iterator<K, V, H>> result = lookup(new_node->hash);
      size_t i = hash_idx(new_node->hash);

      std::unique_lock lock{mutexes[i], std::defer_lock};
      if (T == thread::safe)
        lock.lock();

      if (result.first)
      {
        result.second->second = new_node->second;
        return;
      }

      iterator<K, V, H> tmp = nodes[i];

      if (!tmp)
        nodes[i] = new_node;

      while (tmp)
      {
        if (!tmp->next_bucket)
        {
          tmp->next_bucket = new_node;
          tmp = tmp->next_bucket;
          break;
        }
        else
          tmp = tmp->next_bucket;
      }

      if (!head)
      {
        head = new_node;
        tail = new_node;
      }
      else
      {
        tail->next = new_node;
        tail = tail->next;
      }

      occupied++;
      double load_ratio = ((double)occupied / (double)capacity) * 100;
      uint16_t conv_load = (uint16_t)load_ratio;
      if (conv_load > LOAD_FACTOR)
      {
        if (T == thread::safe)
          lock.unlock();

        extend();
      }
    }

    bool erase(size_t &hash_val)
    {
      if (T == thread::safe)
        ext_wait();

      size_t i = hash_idx(hash_val);

      if (T == thread::safe)
        std::unique_lock lock(mutexes[i]);

      iterator<K, V, H> node = nodes[i];

      if (!node)
        return false;

      if (node->hash == hash_val && node->next_bucket)
      {
        nodes[i] = node->next_bucket;
        return true;
      }
      else if (node->hash == hash_val)
      {
        nodes[i] = nullptr;
        return true;
      }

      while (node)
      {
        if (node->next_bucket == nullptr)
          return false;
        else if (node->next_bucket->hash == hash_val)
        {
          node->next_bucket = nullptr;
          return true;
        }
        else
          node = node->next_bucket;
      }

      return false;
    }

    bool exists(size_t &hash_val)
    {
      std::pair<bool, node<K, V, H> *> result = lookup(hash_val);
      return result.first;
    }

    inline iterator<K, V, H> begin() { return head; }
    inline iterator<K, V, H> end() { return tail->next_bucket; }
  };

}

#endif