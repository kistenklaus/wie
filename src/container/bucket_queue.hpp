#pragma once

#include "container/vector.hpp"
#include <concepts>
#include <forward_list>
#include <functional>
#include <limits>
#include <memory>
#include <ranges>
#include <type_traits>

namespace strobe {

template <typename V, typename Key = std::identity>
  requires(std::is_invocable_v<const Key &, const V &> &&
           std::unsigned_integral<
               std::remove_cvref_t<std::invoke_result_t<const Key, const V &>>>)
class BucketQueue {
private:
  using size_type = std::size_t;
  using value_type = V;
  using reference = V &;
  using const_reference = const V &;
  using pointer = V *;
  using const_pointer = const V *;
  struct Node {
    struct Node *next;
    struct Node *prev;
    size_type bucket;
    V value;

    template <typename... Args>
    explicit Node(Args &&...args)
        : next(this), prev(this), value(std::forward<Args>(args)...) {}
  };

public:
  using K = std::remove_cvref_t<std::invoke_result_t<const Key &, const V &>>;
  using handle = void *;

  BucketQueue(const K keyCount = std::numeric_limits<K>::max(),
              const Key &key = {})
      : m_ringSize(static_cast<size_type>(keyCount)),
        m_ring(static_cast<Node **>(std::malloc(m_ringSize * sizeof(Node *)))),
        m_minBucket(0), m_key(key) {
    assert(m_ringSize != 0);
    assert(m_ring != nullptr);
    std::memset(m_ring, 0, m_ringSize * sizeof(Node *));
  }

  handle push(const V &value) { return emplace<const V &>(value); }

  template <typename... Args> handle emplace(Args &&...args) {
    Node *node = emplace_node<Args...>(std::forward<Args>(args)...);
    insert_node(node);
    return reinterpret_cast<handle>(node);
  }

  void decrease_key(handle h, const V &v) {
    decrease_key(h, [&](reference v) { v = v; });
  }

  void decrease_key(handle h, V &&v) {
    decrease_key(h, [&](reference v) { v = std::move(v); });
  }

  template <typename Fn>
    requires(std::is_invocable_v<Fn, V &>)
  void decrease_key(handle h, Fn &&fn) {
    Node *node = reinterpret_cast<Node *>(h);
    linked_erase(node);
    insert_node(node);
  }

  void pop() {}

  const V &top() {}

private:
  void insert_node(Node *node) {
    size_type k = static_cast<size_type>(m_key(node->value));
    size_type b = k % m_ringSize;
    node->bucket = b;
    Node *bucket = m_ring[b];
    if (bucket == nullptr) {
      node->next = node;
      node->prev = node;
      m_ring[b] = node;
    } else {
      linked_insert_after(bucket, node);
    }
  }

  void linked_insert_after(Node *list, Node *node) {
    Node *next = list->next;
    list->next = node;
    node->prev = list;
    node->next = next;
    next->prev = node;
  }

  void linked_erase(Node *node) {
    Node *prev = node->prev;
    Node *next = node->next;
    if (prev == node) {
      m_ring[node->bucket] = nullptr;
    } else {
      prev->next = next;
      next->prev = prev;

      if (m_ring[node->bucket] == node) {
        m_ring[node->bucket] = next;
      }
    }
#ifndef NDEBUG
    m_ring[node->bucket] = std::numeric_limits<size_type>::max();
    node->prev = node;
    node->next = node;
#endif
  }

  template <typename... Args> Node *emplace_node(Args &&...args) {
    Node *node = alloc_node();
    std::construct_at(node, std::forward<Args>(args)...);
    return node;
  }

  void destroy_node(Node *node) {
    std::destroy_at(node);
    free_node(node);
  }

  Node *alloc_node() {
    Node *node = static_cast<Node *>(std::malloc(sizeof(Node)));
    assert(node != nullptr);
    return node;
  }

  void free_node(Node *node) { std::free(node); }

private:
  size_type m_ringSize;
  Node **m_ring;
  size_type m_minBucket;
  [[no_unique_address]] Key m_key;
};

static void foo() {
  BucketQueue<unsigned int> queue(10); //
}

} // namespace strobe
