#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <functional>
#include <type_traits>
#include <utility>

template <typename T, typename Tag> struct DefaultApply {
  T operator()(T const &value, size_t length, Tag const &tag) const {
    return value + tag * length;
  }
};

template <typename T, typename Tag = T, typename Combine = std::plus<T>,
          typename Compose = Combine, typename Apply = DefaultApply<T, Tag>>
  requires(std::is_default_constructible_v<T> &&
           std::is_default_constructible_v<Tag> &&
           std::is_invocable_r_v<T, Combine, T, T> &&
           std::is_invocable_r_v<Tag, Compose, Tag, Tag> &&
           std::is_invocable_r_v<T, Apply, const T &, size_t, const Tag &>)
class LazySegmentTree {
public:
  using size_type = size_t;
  using value_type = T;
  using tag_type = Tag;

private:
  struct Node {
    T value;
    Tag lazy;
  };

  struct UpdateFrame {
    size_type idx, nl, nr;
    bool after_children;
  };

  struct QueryFrame {
    size_type idx, nl, nr;
  };

public:
  LazySegmentTree(size_type n, Combine combine = {}, Compose compose = {},
                  Apply apply = {})
      : m_size(n), m_combine(std::move(combine)), m_compose(std::move(compose)),
        m_apply(std::move(apply)) {
    if (m_size == 0)
      return;
    const size_type cap = std::bit_ceil(m_size);
    m_nodes = static_cast<Node *>(std::malloc((2 * cap - 1) * sizeof(Node)));
    for (size_type i = 0; i < 2 * cap - 1; ++i)
      std::construct_at(&m_nodes[i], Node{T{}, Tag{}});
  }

  template <typename It>
  LazySegmentTree(It begin, It end, Combine combine = {}, Compose compose = {},
                  Apply apply = {})
      : LazySegmentTree(static_cast<size_type>(std::distance(begin, end)),
                        std::move(combine), std::move(compose),
                        std::move(apply)) {
    if (m_size == 0)
      return;

    const size_type cap = std::bit_ceil(m_size);
    const size_type base = cap - 1;

    auto it = begin;
    for (size_type i = 0; i < m_size; ++i)
      m_nodes[base + i].value = *it++;

    for (ptrdiff_t i = base - 1; i >= 0; --i)
      m_nodes[i].value =
          m_combine(m_nodes[left(i)].value, m_nodes[right(i)].value);
  }

  ~LazySegmentTree() {
    if (m_nodes) {
      const size_type cap = std::bit_ceil(m_size);
      std::destroy_n(m_nodes, 2 * cap - 1);
      std::free(m_nodes);
    }
  }

  LazySegmentTree(const LazySegmentTree &) = delete;
  LazySegmentTree &operator=(const LazySegmentTree &) = delete;

  LazySegmentTree(LazySegmentTree &&o) noexcept
      : m_nodes(std::exchange(o.m_nodes, nullptr)),
        m_size(std::exchange(o.m_size, 0)), m_combine(std::move(o.m_combine)),
        m_compose(std::move(o.m_compose)), m_apply(std::move(o.m_apply)) {}

  LazySegmentTree &operator=(LazySegmentTree &&o) noexcept {
    if (this == &o)
      return *this;
    this->~LazySegmentTree();
    new (this) LazySegmentTree(std::move(o));
    return *this;
  }

  void range_update(size_type l, size_type r, const Tag &tag) {
    if (l >= r || m_size == 0)
      return;

    const size_type cap = std::bit_ceil(m_size);
    UpdateFrame stack[64];
    size_type sp = 0;
    stack[sp++] = {0, 0, cap, false};

    while (sp) {
      UpdateFrame f = stack[--sp];
      size_type i = f.idx;
      size_type nl = f.nl;
      size_type nr = f.nr;

      if (r <= nl || nr <= l)
        continue;

      Node &node = m_nodes[i];
      if (l <= nl && nr <= r) {
        node.value = m_apply(node.value, nr - nl, tag);
        node.lazy = m_compose(node.lazy, tag);
        continue;
      }

      if (!f.after_children) {
        push_lazy(i, nl, nr);
        f.after_children = true;
        stack[sp++] = f; // re-push for post-child combine
        size_type mid = nl + (nr - nl) / 2;
        stack[sp++] = {right(i), mid, nr, false};
        stack[sp++] = {left(i), nl, mid, false};
      } else {
        node.value = m_combine(m_nodes[left(i)].value, m_nodes[right(i)].value);
      }
    }
  }

  T range_query(size_type l, size_type r) {
    if (l >= r || m_size == 0)
      return T{};

    const size_type cap = std::bit_ceil(m_size);
    QueryFrame stack[64];
    size_type sp = 0;
    stack[sp++] = {0, 0, cap};

    T acc{};

    while (sp) {
      QueryFrame f = stack[--sp];
      size_type i = f.idx;
      size_type nl = f.nl;
      size_type nr = f.nr;

      if (r <= nl || nr <= l)
        continue;
      Node &node = m_nodes[i];
      if (l <= nl && nr <= r) {
        acc = m_combine(acc, node.value);
        continue;
      }

      push_lazy(i, nl, nr);
      size_type mid = nl + (nr - nl) / 2;
      stack[sp++] = {right(i), mid, nr};
      stack[sp++] = {left(i), nl, mid};
    }

    return acc;
  }

  size_type size() const noexcept { return m_size; }

private:
  static constexpr size_type left(size_type i) noexcept { return 2 * i + 1; }
  static constexpr size_type right(size_type i) noexcept { return 2 * i + 2; }

  void push_lazy(size_type i, size_type nl, size_type nr) {
    Node &node = m_nodes[i];
    Tag &lazy = node.lazy;
    if (lazy == Tag{})
      return;
    size_type mid = nl + (nr - nl) / 2;
    apply_to_node(left(i), nl, mid, lazy);
    apply_to_node(right(i), mid, nr, lazy);
    lazy = Tag{};
  }

  void apply_to_node(size_type i, size_type nl, size_type nr, const Tag &tag) {
    Node &n = m_nodes[i];
    n.value = m_apply(n.value, nr - nl, tag);
    n.lazy = m_compose(n.lazy, tag);
  }

private:
  Node *m_nodes = nullptr;
  size_type m_size = 0;
  [[no_unique_address]] Combine m_combine;
  [[no_unique_address]] Compose m_compose;
  [[no_unique_address]] Apply m_apply;
};
