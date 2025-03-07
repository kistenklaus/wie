#include <algorithm>
#include <concepts>
#include <iostream>
#include <iterator>
#include <memory>
#include <ranges>
#include <type_traits>
#include <unistd.h>

namespace wie {

template <typename T, typename Allocator = std::allocator<T>>
  requires(
      std::is_same_v<T, typename std::allocator_traits<Allocator>::value_type>)
struct SinglyLinkedList {
private:
  struct Node;
  using allocator_type = Allocator;
  using allocator_traits = std::allocator_traits<allocator_type>;
  using node_allocator_type = allocator_traits::template rebind_alloc<Node>;
  using node_allocator_traits = std::allocator_traits<node_allocator_type>;
  using node_pointer = node_allocator_traits::pointer;
  using node_pointer_traits = std::pointer_traits<node_pointer>;
  using value_type = T;
  using pointer = allocator_traits::pointer;
  using pointer_traits = std::pointer_traits<pointer>;
  using const_pointer = const pointer;
  using reference = value_type &;
  using const_reference = const value_type &;

  struct Node {
    value_type m_value;
    node_pointer m_next;
  };

  struct NodeIterator {
    using iterator_category = std::forward_iterator_tag;
    using value_type = Node;
    using pointer = node_allocator_traits::pointer;
    using pointer_traits = std::pointer_traits<pointer>;
    using reference = value_type &;
    using difference_type = ptrdiff_t;

  public:
    NodeIterator() : m_p() {}
    explicit NodeIterator(node_pointer p) noexcept : m_p(p) {}

    reference operator*() const { return *m_p; }
    pointer operator->() const { return m_p; }

    NodeIterator &operator++() {
      m_p = m_p->m_next;
      return *this;
    }
    NodeIterator operator++(int) {
      NodeIterator it = *this;
      m_p = m_p->m_next;
      return it;
    }

    friend bool operator==(const NodeIterator &a, const NodeIterator &b) {
      return a.m_p == b.m_p;
    }
    friend bool operator!=(const NodeIterator &a, const NodeIterator &b) {
      return a.m_p != b.m_p;
    }

  public:
    node_pointer m_p;
  };

public:
  SinglyLinkedList() : m_allocator{}, m_head{} {}
  explicit SinglyLinkedList(Allocator allocator) : m_allocator(allocator) {}

  SinglyLinkedList<T, Allocator> &
  operator=(const SinglyLinkedList<T, Allocator> &&o) noexcept {
    if (this == &o) {
      return *this;
    }
  }

  void push_front(value_type value) {
    node_pointer second = m_head;
    m_head = node_allocator_traits::allocate(m_allocator, 1);
    m_head->m_value = value;
    m_head->m_next = second;
    m_size += 1;
  }

  void pop_front() {
    assert(m_head != nullptr);
    node_pointer second = m_head->m_next;

    node_allocator_traits::deallocate(m_allocator, m_head, 1);
    m_head = second;
    m_size -= 1;
  }

  reference front() { return m_head->m_value; }

  reference front() const { return m_head->m_value; }

  bool empty() const { return m_head == nullptr; }


  struct Iterator {
    using iterator_category = std::forward_iterator_tag;
    using value_type = SinglyLinkedList::value_type;
    using pointer = SinglyLinkedList::pointer;
    using pointer_traits = std::pointer_traits<pointer>;
    using reference = SinglyLinkedList::reference;
    using difference_type = ptrdiff_t;

  public:
    Iterator() {}
    explicit Iterator(node_pointer p) noexcept : m_it{p} {}
    explicit Iterator(NodeIterator it) noexcept : m_it(it) {}
    reference operator*() const { return m_it->m_value; }
    pointer operator->() const {
      return pointer_traits::pointer_to(m_it->m_value);
    }
    Iterator &operator++() {
      m_it++;
      return *this;
    }
    Iterator operator++(int) {
      Iterator prev = *this;
      m_it++;
      return *this;
    }
    friend bool operator==(const Iterator &a, const Iterator &b) {
      return a.m_it == b.m_it;
    }
    friend bool operator!=(const Iterator &a, const Iterator &b) {
      return a.m_it != b.m_it;
    }

  private:
    NodeIterator m_it;
    friend SinglyLinkedList;
  };
  using iterator = Iterator;

  iterator begin() { return Iterator(m_head); }
  iterator end() { return Iterator(); }

  struct ConstIterator {
    using iterator_category = std::forward_iterator_tag;
    using value_type = SinglyLinkedList::value_type;
    using pointer = SinglyLinkedList::const_pointer;
    using pointer_traits = std::pointer_traits<pointer>;
    using reference = SinglyLinkedList::const_reference;
    using difference_type = ptrdiff_t;

  public:
    ConstIterator() {}
    explicit ConstIterator(node_pointer p) noexcept : m_it{p} {}
    reference operator*() const { return m_it->m_value; }
    pointer operator->() const {
      return pointer_traits::pointer_to(m_it->m_value);
    }
    ConstIterator &operator++() {
      m_it++;
      return *this;
    }
    ConstIterator operator++(int) {
      ConstIterator prev = *this;
      m_it++;
      return *this;
    }
    friend bool operator==(const ConstIterator &a, const ConstIterator &b) {
      return a.m_it == b.m_it;
    }
    friend bool operator!=(const ConstIterator &a, const ConstIterator &b) {
      return a.m_it != b.m_it;
    }

  private:
    NodeIterator m_it;
  };

  using const_iterator = ConstIterator;

  const_iterator begin() const { return ConstIterator(m_head); }
  const_iterator end() const { return ConstIterator(); }
  const_iterator cbegin() const { return ConstIterator(m_head); }
  const_iterator cend() const { return ConstIterator(); }

  template <std::ranges::input_range R>
    requires std::same_as<std::ranges::range_reference_t<R>, reference>
  void insert_range_after(iterator it, const R &rg) {
    insert_range_after(it.m_it, rg);
  }

  void insert_after(const iterator it, value_type v) {
    insert_after(it.m_it, v);
  }


  template <std::ranges::input_range R>
    requires std::same_as<std::ranges::range_reference_t<R>, reference>
  void prepend_range(const R& rg) {
    if (std::ranges::empty(rg)) {
      return;
    }else {
      auto rit = rg.begin();
      node_pointer new_head = node_allocator_traits::allocate(m_allocator, 1);
      new_head->m_value = *rit++;
      new_head->m_next = m_head;
      m_head = new_head;
      insert_range_after(NodeIterator(new_head), std::ranges::subrange(rit, rg.end()));
    }
  }

  void clear() {
    while (m_head != nullptr) {
      node_pointer n = m_head;
      m_head = m_head->m_next;
      node_allocator_traits::deallocate(m_allocator, n, 1);
    }
  }

  template <std::ranges::input_range R>
    requires std::same_as<std::ranges::range_reference_t<R>, reference>
  void assign_range(const R& rg) {
    if (std::ranges::empty(rg)){
      clear();
    }
    
  }


private:
  template <std::ranges::input_range R>
    requires std::same_as<std::ranges::range_reference_t<R>, reference>
  void insert_range_after(NodeIterator it, const R &rg) {
    assert(it != NodeIterator());
    NodeIterator after = it;
    ++after;
    for (const auto &v : rg) {
      node_pointer n = node_allocator_traits::allocate(m_allocator, 1);
      n->m_value = v;
      it->m_next = n;
      ++it;
      assert(it != NodeIterator());
    }
    it->m_next = node_pointer_traits::pointer_to(*after);
  }

  void insert_after(NodeIterator it, value_type v) {
    node_pointer n = node_allocator_traits::allocate(m_allocator, 1);
    n->m_value = v;
    n->m_next = it->m_next;
    it->m_next = n;
  }


  void clear_after(NodeIterator it) {
    node_pointer p = it.m_p->m_next;
    it->m_next = node_pointer();
    while (p != nullptr) {
      node_pointer n = m_head;
      p = p->m_next;
      node_allocator_traits::deallocate(m_allocator, n, 1);
    }
  }

  void erase_after(NodeIterator it) {
    NodeIterator next = it;
    NodeIterator p = ++next;
    if (p == NodeIterator()){
      return;
    }
    ++next;
    it->m_next = next.m_p;
    node_allocator_traits::deallocate(m_allocator, p.m_p, 1);
  }

  template <std::ranges::input_range R>
    requires std::same_as<std::ranges::range_reference_t<R>, reference>
  void overwrite(NodeIterator it) {
  }

  node_allocator_type m_allocator;
  node_pointer m_head;
  size_t m_size;
};

} // namespace wie
