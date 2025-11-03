#pragma once

#include "container/vector.hpp"
#include <bit>
#include <functional>
#include <type_traits>

namespace strobe {

template <typename T, typename Container = strobe::Vector<T>,
          typename Compare = std::less<typename Container::value_type>>
class BinaryHeap {
public:
  using container = Container;
  using comparator = Compare;
  using value_type = typename Container::value_type;
  using reference = typename Container::reference;
  using const_reference = typename Container::const_reference;
  using size_type = typename Container::size_type;
  using difference_type = typename Container::difference_type;

  BinaryHeap()
    requires(std::is_default_constructible_v<container> &&
             std::is_default_constructible_v<comparator>)
      : m_container(), m_comparator() {}
  BinaryHeap(const BinaryHeap &) = default;
  BinaryHeap(BinaryHeap &&) = default;
  BinaryHeap &operator=(const BinaryHeap &) = default;
  BinaryHeap &operator=(BinaryHeap &&) = default;

  const_reference top() const { return m_container.front(); }

  bool empty() const { return std::ranges::empty(m_container); }

  size_type size() const { return std::ranges::size(m_container); }

  void push(const value_type &lvalue) {
    m_container.push_back(lvalue);
    bubbleUp(m_container.size() - 1);
  }

  void push(value_type &&rvalue) {
    m_container.push_back(std::move(rvalue));
    bubbleUp(m_container.size() - 1);
  }

  template <typename... Args> void emplace(Args &&...args) {
    m_container.template emplace_back<Args...>(std::forward<Args>(args)...);
    bubbleUp(m_container.size() - 1);
  }

  void pop() {
    std::swap(m_container.front(), m_container.back());
    m_container.pop_back();
    bubbleDown(0);
  }
  
  void reserve(std::size_t capacity) {
    m_container.reserve(capacity);
  }

private:
  void bubbleUp(size_type index) {
    std::size_t parent = (index - 1) / 2; // may underflow.
    while (index != 0 &&
           m_comparator(m_container[index], m_container[parent])) {
      std::swap(m_container[index], m_container[parent]);
      index = parent;
    }
  }

  void bubbleDown(size_type index) {
    while (index < m_container.size()) {
      std::size_t left = 2 * index + 1;
      std::size_t right = left + 1;
      if (left >= m_container.size()) {
        break;
      }
      size_type next;
      if (right < m_container.size()) {
        next =
            m_comparator(m_container[left], m_container[right]) ? left : right;
      } else {
        next = left;
      }
      if (m_comparator(m_container[index], m_container[next])) {
        break;
      }
      std::swap(m_container[next], m_container[index]);
      index = next;
    }
  }

  container m_container;
  [[no_unique_address]] comparator m_comparator;
};

} // namespace strobe
