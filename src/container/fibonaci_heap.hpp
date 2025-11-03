#pragma once

#include "memory/FreelistPool.hpp"
#include "memory/Mallocator.hpp"
#include <atomic>
#include <concepts>
#include <functional>
#include <memory>
namespace strobe {

template <typename T, typename Compare = std::less<T>> class FibonaciHeap {
private:
  using rank_t = std::uint64_t;
  struct Node {
    T data;
    rank_t rank;
    struct Node *parent;
    struct Node *left;
    struct Node *right;
    struct Node *child;
  };

public:
  using comparator = Compare;
  using value_type = T;
  using reference = T &;
  using const_reference = const T &;
  using pointer = T *;
  using const_pointer = const T *;

  template <typename... Args>
    requires(std::constructible_from<T, Args...>)
  void insert(Args &&...args) {
    Node* node = emplace_node<Args...>(std::forward<Args>(args)...);
    cut(node);
  }

  bool empty() const { return m_root == nullptr; }

private:
  static constexpr rank_t MARK_BIT = rank_t(1) << (sizeof(rank_t) * 8 - 1);
  inline bool is_marked(const Node *node) { return node->rank & MARK_BIT; }
  inline void mark(Node *node) { node->rank |= MARK_BIT; }

  void *cut(Node *node) {
    if (m_root == nullptr) {
    }
  }

  void push_tree(Node* node) {
    if (m_root == nullptr) {

    }
  }

  template <typename... Args>
    requires std::constructible_from<T, Args...>
  Node *emplace_node(Args &&...args) {
    Node *node = alloc_node();
    return std::construct_at(node, std::forward<Args>(args)..., 0, nullptr, node, node,
                      nullptr);
  }

  Node *alloc_node() {
    Node *node = static_cast<Node *>(std::malloc(sizeof(Node)));
    assert(node != nullptr);
    return node;
  }

  void free_node(Node *node) { std::free(node); }

public:
private:
  Node *m_root;
};

} // namespace strobe
