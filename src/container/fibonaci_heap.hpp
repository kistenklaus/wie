#pragma once

#include "memory/FreelistPool.hpp"
#include "memory/Mallocator.hpp"
#include <atomic>
#include <cmath>
#include <concepts>
#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <type_traits>
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

    template <typename... Args>
    explicit Node(Args &&...args)
        : data(std::forward<Args>(args)...), rank(0), parent(nullptr),
          left(nullptr), right(nullptr), child(nullptr) {}
  };

public:
  using comparator = Compare;
  using value_type = T;
  using reference = T &;
  using const_reference = const T &;
  using pointer = T *;
  using const_pointer = const T *;

  using handle = void *;

  FibonaciHeap() : m_root(nullptr) {}

  template <typename... Args>
    requires(std::constructible_from<T, Args...>)
  handle emplace(Args &&...args) {
    Node *node = emplace_node<Args...>(std::forward<Args>(args)...);
    push_tree(node);
    return reinterpret_cast<void *>(node);
  }
  handle push(const value_type &v) { return emplace<const value_type &>(v); }
  handle push(value_type &&v) { return emplace<value_type &&>(std::move(v)); }
  bool empty() const { return m_root == nullptr; }
  void pop() {
    assert(m_root != nullptr);
    Node *node = m_root;

    // all all childs as new trees.
    Node *child = m_root->child;
    if (child != nullptr) {
      Node *curr = child;
      do {
        Node *next = curr->right;
        push_tree(curr);
        curr = next;
      } while (curr != child);
    }

    // delete min from forest.
    linked_erase(m_root);
    destroy_node(node);
    rebuild();
  }

  const_reference top() const { return m_root->data; }

  void decrease_key(handle h, const value_type &new_value) {
    decrease_key(h, [&](value_type &value) { value = new_value; });
  }

  void decrease_key(handle h, value_type &&new_value) {
    decrease_key(h, [&](value_type &value) { value = std::move(new_value); });
  }

  template <typename Func>
    requires std::is_invocable_v<Func, value_type &>
  void decrease_key(handle h, Func func) {
    assert(h != nullptr);
    Node *node = reinterpret_cast<Node *>(h);
    func(node->data);
    if (node->parent == nullptr) {
      if (comparator{}(node->data, m_root->data)) {
        m_root = node;
      }
    } else {
      if (comparator{}(node->data, node->parent->data)) {
        cut(node);
      }
    }
  }

  void erase(handle h) {
    Node *node = reinterpret_cast<Node *>(h);
    if (node == m_root) {
      pop();
    } else {
      Node *child = node->child;
      if (child != nullptr) {
        Node *curr = child;
        do {
          Node *next = curr->right;
          push_tree(curr);
          curr = next;
        } while (curr != child);
      }
      linked_erase(node);
      destroy_node(node);
    }
  }

private:
  static constexpr rank_t MARK_BIT = rank_t(1) << (sizeof(rank_t) * 8 - 1);
  inline bool is_marked(const Node *node) { return node->rank & MARK_BIT; }
  inline void mark(Node *node) { node->rank |= MARK_BIT; }

  void cut(Node *node) {
    linked_erase(node);
    push_tree(node);
  }

  Node *union_trees(Node *lhs, Node *rhs) {
    assert(lhs->parent == nullptr);
    assert(rhs->parent == nullptr);
    bool c = comparator{}(lhs->data, rhs->data);
    Node *parent = c ? lhs : rhs;
    Node *child = c ? rhs : lhs;
    link(parent, child);
    return parent;
  }

  void link(Node *parent, Node *child) {
    assert(parent != nullptr);
    assert(child != nullptr);
    assert(parent->parent == nullptr);
    assert(child->parent == nullptr);
    linked_erase(child);
    linked_insert_child(parent, child);
  }

  void push_tree(Node *node) {
    node->parent = nullptr;
    if (m_root == nullptr) {
      node->left = node;
      node->right = node;
      m_root = node;
    } else {
      linked_insert_after(m_root, node);
      if (comparator{}(node->data, m_root->data)) {
        m_root = node;
      }
    }
  }

  // NOTE: Does not delete the node.
  void linked_erase(Node *node) {
    Node *left = node->left;
    Node *right = node->right;
    if (left == node) {
      if (node->parent == nullptr) {
        m_root = nullptr;
      } else {
        node->parent->child = nullptr;
      }
    } else {
      left->right = right;
      right->left = left;
      if (node->parent == nullptr && m_root == node) {
        m_root = right;
      } else if (node->parent != nullptr && node->parent->child == node) {
        node->parent->child = left;
      }
    }
    node->left = nullptr;
    node->right = nullptr;
  }

  void linked_insert_child(Node *parent, Node *node) {
    if (parent->child == nullptr) {
      parent->child = node;
      node->left = node;
      node->right = node;
      node->parent = parent;
    } else {
      Node *child = parent->child;
      linked_insert_after(child, node);
      node->parent = parent;
    }
  }

  void linked_insert_after(Node *pos, Node *node) {
    assert(pos != nullptr);
    Node *right = pos->right;
    Node *left = pos;
    left->right = node;
    node->left = left;
    right->left = node;
    node->right = right;
  }

  void rebuild() {
    // pairwise union, implicitly finds the min.
    if (m_root == nullptr) {
      return;
    }
    Node *head = m_root;
    Node *next = head->right;
    while (head != next) {
      head = union_trees(head, next);
      next = head->right;
    }
  }

  template <typename... Args>
    requires std::constructible_from<T, Args...>
  Node *emplace_node(Args &&...args) {
    Node *node = alloc_node();
    std::construct_at(node, std::forward<Args>(args)...);
    return node;
  }

  Node *alloc_node() {
    Node *node = static_cast<Node *>(std::malloc(sizeof(Node)));
    assert(node != nullptr);
    return node;
  }

  void destroy_node(Node *node) {
    std::destroy_at(node);
    free_node(node);
  }

  void free_node(Node *node) { std::free(node); }

public:
private:
  Node *m_root;
};

} // namespace strobe
