#ifndef DYNA_NODE_HPP
#define DYNA_NODE_HPP

#include <stdint.h>
#include <utility>

namespace dyna
{

  template <typename K, typename H>
  size_t hash_func(K &key)
  {
    H hasher;
    return hasher(key);
  }

  template <typename K, typename V, typename H>
  struct node : public std::pair<K *, V *>
  {
    node<K, V, H> *next;
    node<K, V, H> *prev;

    node<K, V, H> *next_bucket;
    node<K, V, H> *prev_bucket;

    node<K, V, H> *next_subtable;
    node<K, V, H> *prev_subtable;

    size_t *hash;

    node() : std::pair<K *, V *>(nullptr, nullptr)
    {
      next = nullptr;
      next_bucket = nullptr;
      hash = nullptr;
    }

    node(K key, V value) : std::pair<K *, V *>(new K(key), new V(value))
    {
      hash = new size_t(hash_func<K, H>(key));
      next = nullptr;
      next_bucket = nullptr;
    }

    node(K key, V value, size_t hash_val) : std::pair<K *, V *>(new K(key), new V(value))
    {
      hash = new size_t(hash_val);
      next = nullptr;
      next_bucket = nullptr;
    }

    ~node()
    {
      next = nullptr;
      next_bucket = nullptr;
    }

    inline bool operator==(node<K, V, H> &other)
    {
      return this->first == other.first &&
             this->second == other.second;
    }

    inline bool empty()
    {
      return !this->first && !this->second;
    }
  };

  template <typename K, typename V, typename H>
  using iterator = node<K, V, H> *;
}

#endif