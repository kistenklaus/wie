#pragma once

#include <functional>
#include <limits>
#include "container/vector.hpp"

namespace strobe {

template <typename T, std::size_t K, typename Container = strobe::Vector<T>,
          typename Compare = std::less<typename Container::value_type>>
class KAryHeap {
public:
  using container = Container;
  using comparator = Compare;
  using value_type = typename Container::value_type;
  using reference = typename Container::reference;
  using const_reference = typename Container::const_reference;
  using size_type = typename Container::size_type;
  using difference_type = typename Container::difference_type;

  KAryHeap()
    requires(std::is_default_constructible_v<container> &&
             std::is_default_constructible_v<comparator>)
      : m_container(), m_comparator() {}
  KAryHeap(const KAryHeap &) = default;
  KAryHeap(KAryHeap &&) = default;
  KAryHeap &operator=(const KAryHeap &) = default;
  KAryHeap &operator=(KAryHeap &&) = default;

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
    std::size_t parent = (index - 1) / K; // may underflow.
    while (index != 0 &&
           m_comparator(m_container[index], m_container[parent])) {
      std::swap(m_container[index], m_container[parent]);
      index = parent;
    }
  }

  void bubbleDown(size_type index) {
    while (index < m_container.size()) {
      size_type left = index * K + 1;
      size_type right = left + K - 1;
      size_type end = std::min(right, m_container.size() - 1);
      size_type next = index;
      for (size_type n = left; n < end; ++n) {
        if (m_comparator(m_container[n], m_container[next])) {
          next = n;
        }
      }
      if (next == index) {
        break;
      }
      std::swap(m_container[next], m_container[index]);
      index = next;
    }
  }

private:
  container m_container;
  [[no_unique_address]] comparator m_comparator;
};

} // namespace strobe
