#include <bit>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <memory>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <vector>

template <typename T, typename Op = std::plus<T>>
  requires(std::is_invocable_r_v<T, Op, T, T>)
class EagerSegmentTree {
public:
  using value_type = T;
  using size_type = size_t;
  using difference_type = ptrdiff_t;

  template <typename It> EagerSegmentTree(It begin, It end) {
    m_size = static_cast<size_type>(std::distance(begin, end));
    if (m_size == 0) {
      m_buffer = nullptr;
      return;
    }

    size_type capacity = 2 * m_size - 1;
    m_buffer = static_cast<T *>(std::malloc(capacity * sizeof(T)));

    size_type elem_last_row = 2 * m_size - std::bit_ceil(m_size);
    size_type last_row = std::bit_ceil(m_size) - 1;
    auto it = begin;
    std::uninitialized_copy_n(it, elem_last_row, m_buffer + last_row);
    std::advance(it, elem_last_row);
    std::uninitialized_copy(it, end, m_buffer + m_size - 1);

    for (difference_type i = static_cast<difference_type>(m_size) - 2; i >= 0;
         --i) {
      std::construct_at(m_buffer + i,
                        Op{}(m_buffer[left(i)], m_buffer[right(i)]));
    }
  }

  ~EagerSegmentTree() {
    if (m_buffer != nullptr) {
      size_type capacity = 2 * m_size - 1;
      std::destroy_n(m_buffer, capacity);
      std::free(m_buffer);
      m_size = 0;
    }
  }

  const T &operator[](size_type index) const {
    assert(index < m_size);
    size_type node = m_size - 1 + index;
    return m_buffer[node];
  }

  const T &at(size_type index) const {
    if (index >= m_size) {
      throw std::out_of_range("index is out of range");
    }
    size_type node = leaf_node(index);
    return m_buffer[node];
  }

  void set(size_type index, const T &value) {
    assert(index < m_size);
    size_type node = leaf_node(index);
    m_buffer[node] = value;
    while (node > 0) {
      node = parent(node);
      m_buffer[node] = Op{}(m_buffer[left(node)], m_buffer[right(node)]);
    }
  }

  void update(size_type index, const T &delta) {
    assert(index < m_size);
    size_type node = leaf_node(index);
    m_buffer[node] = Op{}(m_buffer[node], delta);
    while (node > 0) {
      node = parent(node);
      m_buffer[node] = Op{}(m_buffer[left(node)], m_buffer[right(node)]);
    }
  }

  void lhs_update(size_type index, const T &delta) {
    assert(index < m_size);
    size_type node = leaf_node(index);
    m_buffer[node] = Op{}(delta, m_buffer[node]);
    while (node > 0) {
      node = parent(node);
      m_buffer[node] = Op{}(m_buffer[left(node)], m_buffer[right(node)]);
    }
  }

  T range_query(size_type l, size_type r) const {
    assert(l < r && r <= m_size);
    size_type left = leaf_node(l);
    size_type right = leaf_node(r - 1) + 1; // exclusive

    std::optional<T> left_acc;
    std::optional<T> right_acc;

    if (left >= right) {
      if ((left & 1u) == 0u) {
        left_acc.emplace(m_buffer[left]);
        ++left;
      }
      left = parent(left);
      //right = parent(right - 1) + 1;
    }

    while (left < right) {
      if ((left & 1u) == 0u) {
        if (left_acc)
          *left_acc = Op{}(*left_acc, m_buffer[left]);
        else
          left_acc.emplace(m_buffer[left]);
        ++left;
      }

      if ((right & 1u) == 0u) {
        --right;
        if (right_acc)
          *right_acc = Op{}(m_buffer[right], *right_acc);
        else
          right_acc.emplace(m_buffer[right]);
      }

      if (left == right)
        break;

      left = parent(left);
      right = parent(right);
    }

    if (left_acc && right_acc)
      return Op{}(*left_acc, *right_acc);
    else if (left_acc)
      return *left_acc;
    else if (right_acc)
      return *right_acc;
    else
      return m_buffer[0];
  }

  void range_update(size_type l, size_type r, const T &delta) {
    if (l == r) {
      return;
    }
    assert(l < r);
    size_type left_node = leaf_node(l);
    size_type right_node = leaf_node(r - 1) + 1;

    if (left_node >= right_node) {
      for (size_type i = left_node; i < 2 * m_size - 1; ++i) {
        m_buffer[i] = Op{}(m_buffer[i], delta);
      }
      left_node = parent(left_node);
      for (size_type i = left_node; i < m_size - 1; ++i) {
        m_buffer[i] = Op{}(m_buffer[left(i)], m_buffer[right(i)]);
      }
      for (size_type i = m_size - 1; i < right_node; ++i) {
        m_buffer[i] = Op{}(m_buffer[i], delta);
      }
    } else {
      for (size_type i = left_node; i < right_node; ++i) {
        m_buffer[i] = Op{}(m_buffer[i], delta);
      }
    }

    while (left_node > 0) {
      left_node = parent(left_node);
      right_node = parent(right_node - 1) + 1;
      for (size_type i = left_node; i < right_node; ++i) {
        m_buffer[i] = Op{}(m_buffer[left(i)], m_buffer[right(i)]);
      }
    }
  }

  /// Requires that if cond(agg) is true, there exists at least one
  /// child which also satisfied cond.
  template <typename Cond>
    requires(std::is_invocable_r_v<bool, Cond, const value_type &>)
  std::optional<size_type> find_first(Cond &&cond) {
    if (m_size == 0) {
      return std::nullopt;
    }
    if (!cond(m_buffer[0])) {
      return std::nullopt;
    }
    size_type node = 0;
    while (node < m_size - 1) {
      size_type left_node = left(node);
      size_type right_node = right(node);

      if (cond(m_buffer[left_node])) {
        node = left_node;
      } else {
        node = right_node;
      }
    }
    size_type elem_last_row = 2 * m_size - std::bit_ceil(m_size);
    if (std::bit_width(m_size) == std::bit_width(node + 1)) {
      return elem_last_row + node - (m_size - 1);
    } else {
      return node - (m_size - 1) - (m_size - elem_last_row);
    }
  }

  /// Requires that if cond(agg) is true, there exists at least one
  /// child which also satisfied cond.
  template <typename Cond> std::optional<size_type> find_last(Cond &&cond) {
    if (m_size == 0) {
      return std::nullopt;
    }
    if (!cond(m_buffer[0])) {
      return std::nullopt;
    }
    size_type node = 0;
    while (node < m_size - 1) {
      size_type left_node = left(node);
      size_type right_node = right(node);
      if (cond(m_buffer[right_node])) {
        node = right_node;
      } else {
        node = left_node;
      }
    }
    size_type elem_last_row = 2 * m_size - std::bit_ceil(m_size);
    if (std::bit_width(m_size) == std::bit_width(node + 1)) {
      return elem_last_row + node - (m_size - 1);
    } else {
      return node - (m_size - 1) - (m_size - elem_last_row);
    }
  }

  bool empty() const { return m_size == 0; }
  size_type size() const { return m_size; }

private:
  static constexpr size_type parent(difference_type index) {
    return (index - 1) / 2;
  }

  static constexpr size_type left(difference_type index) {
    return index * 2 + 1;
  }

  static constexpr size_type right(difference_type index) {
    return index * 2 + 2;
  }

  constexpr size_type leaf_node(size_type index) const {
    size_type elem_last_row = 2 * m_size - std::bit_ceil(m_size);
    if (index < elem_last_row) {
      size_type begin = std::bit_ceil(m_size) - 1;
      return begin + index;
    } else {
      return m_size - 1 + index - elem_last_row;
    }
  }

  T *m_buffer;
  size_type m_size;
};

