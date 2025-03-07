#include <cassert>
#include <cstring>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <memory>
#include <type_traits>

namespace wie {

template <typename T, typename Allocator = std::allocator<T>>
  requires(
      std::is_same_v<typename std::allocator_traits<Allocator>::value_type, T>)
struct Vec {
private:
  using value_type = T;
  using allocator_type = Allocator;
  using allocator_traits = std::allocator_traits<allocator_type>;
  using reference = value_type &;
  using const_reference = const value_type &;
  using pointer = allocator_traits::pointer;
  using const_pointer = allocator_traits::const_pointer;
  using iterator = pointer;
  using const_iterator = const_pointer;

  static_assert(
      std::is_same_v<typename allocator_type::value_type, value_type>);

  static constexpr size_t DEFAULT_CAPACITY = 0;

public:
  explicit Vec() : m_allocator(), m_size(0), m_capacity(0), m_buffer(nullptr) {}

  explicit Vec(const Allocator &allocator)
      : m_allocator(allocator), m_size(0), m_capacity(0), m_buffer(nullptr) {}

  explicit Vec(size_t count)
      : m_allocator(), m_size(count), m_capacity(count),
        m_buffer(allocator_traits::allocate(m_allocator, count)) {
    static_assert(
        std::is_default_constructible_v<value_type>(),
        "Call to Vec<T>::Vec(size_t) requires T to be default constructible");
  }

  explicit Vec(size_t count, const Allocator &allocator)
      : m_allocator(allocator), m_size(count), m_capacity(count),
        m_buffer(allocator_traits::allocate(m_allocator, count)) {
    static_assert(
        std::is_default_constructible_v<value_type>(),
        "Call to Vec<T>::Vec(size_t) requires T to be default constructible");
  }
  explicit Vec(std::initializer_list<T> initalizer_list)
      : m_allocator(), m_size(initalizer_list.size()), m_capacity(m_size),
        m_buffer(m_size != 0 ? allocator_traits::allocate(m_allocator, m_size)
                             : nullptr) {
    std::memcpy(m_buffer, initalizer_list.begin(), m_size);
  }

  template <typename OAlloc>
    requires(!std::is_same_v<OAlloc, Allocator>)
  Vec(const Vec<T, OAlloc> &o) noexcept
      : m_allocator(), m_size(o.size()), m_capacity(m_size),
        m_buffer(allocator_traits::allocate(m_allocator, m_size)) {
    std::memcpy(m_buffer, o.get(), m_size);
  }

  template <typename OAlloc>
    requires(!std::is_same_v<OAlloc, Allocator>)
  Vec(const Vec<T, OAlloc> &o, const Allocator &allocator) noexcept
      : m_allocator(allocator), m_size(o.size()), m_capacity(m_size),
        m_buffer(allocator_traits::allocate(m_allocator, m_size)) {
    std::memcpy(m_buffer, o.get(), m_size);
  }

  Vec(const Vec<T, Allocator> &o) noexcept
      : m_allocator(allocator_traits::select_on_container_copy_construction(
            o.m_allocator)),
        m_size(o.m_size), m_capacity(m_size),
        m_buffer(m_size != 0 ? allocator_traits::allocate(m_allocator, m_size)
                             : nullptr) {
    std::memcpy(m_buffer, o.m_buffer, m_size);
  }

  template <typename OAlloc>
    requires(!std::is_same_v<OAlloc, Allocator>)
  Vec(Vec<T, OAlloc> &&o) = delete;

  Vec(Vec<T, Allocator> &&o)
      : m_allocator(o.m_allocator), m_size(o.m_size), m_capacity(o.m_capacity),
        m_buffer(o.m_buffer) {
    o.m_buffer = nullptr;
    o.m_size = 0;
    o.m_capacity = 0;
  }

  template <typename OAlloc>
    requires(!std::is_same_v<OAlloc, Allocator>)
  Vec<T, Allocator> &operator=(const Vec<T, OAlloc> &o) noexcept {
    m_size = o.size();
    if (m_capacity < m_size) {
      allocator_traits::deallocate(m_allocator, m_buffer, m_capacity);
      m_buffer = allocator_traits::allocate(m_allocator, m_size);
      m_capacity = m_size;
    }
    std::memcpy(m_buffer, o.get(), m_size);

    return *this;
  }

  Vec<T, Allocator> &operator=(const Vec<T, Allocator> &o) noexcept {
    if (this == &o) {
      return *this;
    }
    if constexpr (std::is_same_v<typename allocator_traits::
                                     propagate_on_container_copy_assignment,
                                 std::true_type>) {
      if (m_allocator != o.m_allocator) {
        allocator_traits::deallocate(m_allocator, m_buffer, m_capacity);
        m_allocator = o.m_allocator;
        m_buffer = allocator_traits::allocate(m_allocator, o.m_size);
        m_capacity = o.m_size;
      } else if (m_capacity < o.m_size) {
        allocator_traits::deallocate(m_allocator, m_buffer, m_capacity);
        m_buffer = allocator_traits::allocate(m_allocator, o.m_size);
        m_capacity = o.m_size;
      }
    } else {
      if (m_capacity < o.m_size) {
        allocator_traits::deallocate(m_allocator, m_buffer, m_capacity);
        m_buffer = allocator_traits::allocate(m_allocator, o.m_size);
      }
    }
    m_size = o.m_size;
    std::memcpy(m_buffer, o.m_buffer, m_size);
    return *this;
  }

  template <typename OAlloc>
    requires(!std::is_same_v<OAlloc, Allocator>)
  Vec<T, Allocator> &operator=(Vec<T, OAlloc> &&o) noexcept {
    m_size = o.size();
    if (m_capacity < m_size) {
      allocator_traits::deallocate(m_allocator, m_buffer, m_capacity);
      m_buffer = allocator_traits::allocate(m_allocator, m_size);
      m_capacity = m_size;
    }
    std::memcpy(m_buffer, o.get(), m_size);
    return *this;
  }

  Vec<T, Allocator> &operator=(Vec<T, Allocator> &&o) noexcept {
    if (this == &o) {
      return this;
    }
    m_size = o.m_size;
    if constexpr (std::is_same_v<typename allocator_traits::
                                     propagate_on_container_move_assignment,
                                 std::true_type>) {
      allocator_traits::deallocate(m_allocator, m_buffer, m_capacity);
      m_allocator = o.m_allocator;
      m_buffer = o.m_buffer;
      m_capacity = o.m_capacity;
    } else {
      if (!std::is_same_v<typename allocator_traits::is_always_equals,
                          std::true_type> &&
          m_allocator != o.m_allocator) {
        if (m_capacity < o.m_size) {
          allocator_traits::deallocate(m_allocator, m_buffer, m_capacity);
          m_buffer = allocator_traits::allocate(m_allocator, o.m_size);
          m_capacity = o.m_size;
        }
        std::memcpy(m_buffer, o.m_buffer, o.m_size);
      } else {
        allocator_traits::deallocate(m_allocator, m_buffer, m_capacity);
        m_buffer = o.m_buffer;
        m_capacity = o.m_capacity;
      }
    }
    o.m_buffer = nullptr;
    o.m_size = 0;
    o.m_capacity = 0;
    return *this;
  }

  ~Vec() {
    allocator_traits::deallocate(m_allocator, m_buffer, m_capacity);
#ifndef NDEBUG
    m_buffer = nullptr;
    m_size = 0;
    m_capacity = 0;
#endif
  }

  void push_back(value_type value) {
    if (m_size == m_capacity) {
      grow();
    }
    m_buffer[m_size] = value;
    m_size += 1;
  }

  void pop_back() {
    assert(!empty());
    m_size -= 1;
  }

  bool empty() { return m_size != 0; }

  template <typename... Args> void emplace_back(Args &&...args) {
    if (m_size == m_capacity) {
      grow();
    }
    new (m_buffer + m_size) value_type(std::forward<Args>(args)...);
    m_size += 1;
  }

  reference operator[](size_t index) {
    assert(index < m_size);
    return m_buffer[index];
  }

  const_reference operator[](size_t index) const {
    assert(index < m_size);
    return m_buffer[index];
  }

  reference back() { return m_buffer[m_size - 1]; }
  const_reference back() const { return m_buffer[m_size - 1]; }

  reference front() { return m_buffer[0]; }
  const_reference front() const { return m_buffer[0]; }

  size_t size() const { return m_size; }

  const pointer get() const { return m_buffer; }

  Allocator get_allocator() { return m_allocator; }

  iterator begin() { return m_buffer; }
  const_iterator begin() const { return m_buffer; }

  iterator end() { return m_buffer + m_size; }
  const_iterator end() const { return m_buffer + m_size; }

private:
  void grow() {
    m_capacity = (m_size * 3) / 2 + 1;
    pointer new_buffer = allocator_traits::allocate(m_allocator, m_capacity);
    std::memcpy(new_buffer, m_buffer, m_size * sizeof(value_type));
    allocator_traits::deallocate(m_allocator, m_buffer, m_size);
    m_buffer = new_buffer;
  }

private:
  [[no_unique_address]] allocator_type m_allocator;
  size_t m_size;
  size_t m_capacity;
  pointer m_buffer;
};

} // namespace wie
