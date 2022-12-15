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
      prev_bucket = nullptr;
      next_subtable = nullptr;
      prev_subtable = nullptr;
      hash = nullptr;
    }

    node(K key, V value) : std::pair<K *, V *>(new K(key), new V(value))
    {
      hash = new size_t(hash_func<K, H>(key));
      next = nullptr;
      next_bucket = nullptr;
      prev_bucket = nullptr;
      next_subtable = nullptr;
      prev_subtable = nullptr;
    }

    node(K key, V value, size_t hash_val) : std::pair<K *, V *>(new K(key), new V(value))
    {
      hash = new size_t(hash_val);
      next = nullptr;
      next_bucket = nullptr;
      prev_bucket = nullptr;
      next_subtable = nullptr;
      prev_subtable = nullptr;
    }

    inline bool operator==(node<K, V, H> &other)
    {
      return this->first == other.first &&
             this->second == other.second;
    }

    inline bool empty() { return !this->first && !this->second; }
    inline K &key() { return *this->first; }
    inline K &value() { return *this->second; }
  };

  template <typename K, typename V, typename H> 
  class iterator {
    node<K, V, H> *data;

  public:
    iterator(): data(nullptr) {}
    iterator(node<K, V, H> *data): data(data) {}

    node<K, V, H> *operator->()
    {
      return data;
    }

    iterator &operator++()
    {
      if (this->data)
        this->data = this->data->next;

      return *this;
    }

    iterator &operator++(int)
    {
      if (this->data)
        this->data = this->data->next;

      return *this;
    }

    iterator &operator--()
    {
      if (this->data)
        this->data = this->data->prev;

      return *this;
    }

    void operator+=(size_t add)
    {
      for (int i = 0; i < add; i++)
      {
        if (!this->data && !this->data->next)
        {
          this->data = nullptr;
          return;
        }
        this->data = this->data->next;
      }
    }

    void operator-=(size_t neg)
    {
      for (int i = 0; i < neg; i++)
      {
        if (!this->data && !this->data->prev)
        {
          this->data = nullptr;
          return;
        }
        this->data = this->data->prev;
      }
    }

    bool operator==(iterator &other)
    {
      if (!this->data && !other.data)
        return true;
      
      if (!this->data || !other.data)
        return false;

      return this->data->hash == other.data->hash;
    }

    bool operator!=(iterator &other)
    {
      if (!this->data && !other.data)
        return false;
      
      if (!this->data || !other.data)
        return true;

      return this->data->hash != other.data->hash;
    }

    iterator &operator*() { return *this; }
  };
}

#endif