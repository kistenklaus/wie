#pragma once

#include <bit>
#include <cstdlib>
#include <functional>
#include <new>

template <typename T, typename Op = std::plus<T>,
          typename InvOp = std::minus<T>>
class FenwickTree {
public:
  using size_type = size_t;

  FenwickTree(size_type n, const Op &op = {}, const InvOp &invOp = {})
      : m_op(op), m_invOp(invOp), m_size(n) {
    m_buffer = static_cast<T *>(std::malloc((n + 1) * sizeof(T)));
    for (size_t i = 0; i <= n; ++i) {
      std::construct_at(m_buffer + i, T{});
    }
  }

  ~FenwickTree() {
    std::destroy_n(m_buffer, m_size + 1);
    std::free(m_buffer);
  }

  void update(size_t i, const T &delta) {
    for (++i; i <= m_size; i += i & -i) {
      m_buffer[i] = m_op(m_buffer[i], delta);
    }
  }

  T prefix_query(size_t r) const {
    T res{};
    for (++r; r > 0; r -= r & -r) {
      res = m_op(res, m_buffer[r]);
    }
    return res;
  }

  T range_query(size_t l, size_t r) const {
    return m_invOp(prefix_query(r), prefix_query(l - 1));
  }

  T at(size_t i) const { return range_query(i, i); }

  T operator[](size_t i) const { return at(i); }

  void set(size_t i, const T &value) {
    T cur = at(i);
    T delta = m_invOp(value, cur);
    update(i, delta);
  }

  size_t size() const { return m_size; }

private:
  [[no_unique_address]] Op m_op;
  [[no_unique_address]] InvOp m_invOp;
  size_type m_size;
  T *m_buffer;
};
