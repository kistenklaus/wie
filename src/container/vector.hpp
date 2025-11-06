#pragma once

#include "container/container_concepts.hpp"
#include "memory/AllocatorTraits.hpp"
#include "memory/Mallocator.hpp"
#include "type_traits/trivially_destructible_after_move.hpp"
#include <algorithm>
#include <cassert>
#include <cstring>
#include <iterator>
#include <memory>
#include <ranges>
#include <type_traits>
namespace strobe {

template <typename T, Allocator A = strobe::Mallocator> class Vector {
  using ATraits = AllocatorTraits<A>;

public:
  // NOTE: Required for the competition!
  using value_type = T;
  using allocator_type = A;
  using size_type = std::size_t;
  // NOTE: Optional if we also want to be standard conforming and support the
  // range interface i.e. be iterable!
  using difference_type = std::ptrdiff_t;
  using reference = value_type &;
  using const_reference = const value_type &;
  using pointer = ATraits::template pointer<value_type>;
  using const_pointer = ATraits::template const_pointer<value_type>;
  using iterator = pointer;
  using const_iterator = const_pointer;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  // =================== Constructors =======================
  explicit Vector(const A &alloc = {})
      : m_capacity(0), m_size(0), m_buffer(nullptr), m_allocator(alloc) {}

  explicit Vector(size_type size, const A &alloc = {})
    requires std::is_default_constructible_v<T>
      : m_capacity(size), m_size(size), m_allocator(alloc) {
    m_buffer = ATraits::template allocate<T>(m_allocator, m_capacity);
    assert(m_capacity == 0 || m_buffer != nullptr);
    std::uninitialized_value_construct(m_buffer, m_buffer + size);
  }

  explicit Vector(size_type size, const T &value, const A &alloc = {})
      : m_capacity(size), m_size(size), m_allocator(alloc) {
    m_buffer = ATraits::template allocate<T>(m_allocator, m_capacity);
    assert(m_capacity == 0 || m_buffer != nullptr);
    std::uninitialized_fill(m_buffer, m_buffer + size, value);
  }

  template <std::ranges::range Rg>
    requires(std::same_as<std::ranges::range_value_t<Rg>, value_type>)
  explicit Vector(const Rg &rg, const A &alloc = {}) : m_allocator(alloc) {
    if constexpr (std::ranges::sized_range<Rg>) {
      m_capacity = std::ranges::size(rg);
      m_size = m_capacity;
      m_buffer = ATraits::template allocate<T>(m_allocator, m_capacity);
      assert(m_buffer != nullptr || m_capacity == 0);

      if constexpr (std::ranges::contiguous_range<Rg> &&
                    std::is_trivially_copy_constructible_v<T>) {
        std::memcpy(m_buffer, std::ranges::data(rg), m_capacity * sizeof(T));
      } else {
        /* generic path */
        std::uninitialized_copy(std::ranges::begin(rg), std::ranges::end(rg),
                                m_buffer);
      }
    } else if constexpr (std::ranges::forward_range<Rg>) {
      const size_type n = static_cast<size_type>(std::ranges::distance(rg));
      m_capacity = m_size = n;
      m_buffer = ATraits::template allocate<T>(m_allocator, n);
      assert(m_buffer != nullptr || n == 0);

      if constexpr (std::ranges::contiguous_range<Rg> &&
                    std::is_trivially_copyable_v<T>)
        std::memcpy(m_buffer, std::ranges::data(rg), n * sizeof(T));
      else
        std::uninitialized_copy(std::ranges::begin(rg), std::ranges::end(rg),
                                m_buffer);
    } else {
      for (const T &v : rg)
        push_back(v);
    }
  }

  ~Vector() { reset(); }

  Vector(const Vector &o)
      : m_capacity(o.m_size), m_size(o.m_size),
        m_allocator(
            ATraits::select_on_container_copy_construction(o.m_allocator)) {
    m_buffer = ATraits::template allocate<T>(m_allocator, m_size);
    if (std::is_trivially_copyable_v<T>) {
      std::memcpy(m_buffer, o.m_buffer, sizeof(T) * m_size);
    } else {
      std::uninitialized_copy(o.m_buffer, o.m_buffer + m_size, m_buffer);
    }
  }

  Vector &operator=(const Vector &o) {
    if (this == &o) {
      return *this;
    }

    // NOTE: This is_always_equal means that all instances of a allocator type
    // (A) manage the same memory resource this implies that any instance can
    // be used for allocation / deallocation.
    // Example: Any instance of the Mallocator which uses malloc and free and
    // be used for allocation and deallocation of any buffer.
    const bool equalAllocator =
        strobe::alloc_equals(m_allocator, o.m_allocator);
    if ((ATraits::propagate_on_container_copy_assignment && !equalAllocator) ||
        m_capacity <= o.m_size) {
      // The current instance does not have enough capacity therefor we have
      // Destruct and release current buffer.
      reset();

      if (ATraits::propagate_on_container_copy_assignment && !equalAllocator) {
        m_allocator = o.m_allocator;
      }

      // Allocate new buffer.
      m_buffer = ATraits::template allocate<T>(m_allocator, o.m_size);
      assert(m_capacity == 0 || m_buffer != nullptr);
      m_capacity = o.m_size;
      copy_construct_from(o.m_buffer, o.m_size);
    } else {
      copy_assign_from(o.m_buffer, o.m_size);
    }
    return *this;
  }

  Vector(Vector &&o) noexcept
      : m_capacity(std::exchange(o.m_capacity, 0)),
        m_size(std::exchange(o.m_size, 0)),
        m_buffer(std::exchange(o.m_buffer, nullptr)),
        m_allocator(std::move(o.m_allocator)) {}

  Vector &operator=(Vector &&o) noexcept {
    if (this == &o)
      return *this;

    const bool equalAllocator =
        strobe::alloc_equals(m_allocator, o.m_allocator);

    if (ATraits::propagate_on_container_move_assignment || equalAllocator) {

      // Safe to take over the buffer directly.
      reset();
      m_allocator = std::move(o.m_allocator);
      m_buffer = std::exchange(o.m_buffer, nullptr);
      m_capacity = std::exchange(o.m_capacity, 0);
      m_size = std::exchange(o.m_size, 0);
    } else {
      // Allocators are incompatible, we cannot reuse the buffer of o.
      if (m_capacity < o.m_size) {
        // this buffer also does not have enough capacity we have to
        // reallocate!
        reset();
        m_buffer = ATraits::template allocate<T>(m_allocator, o.m_size);
        assert(m_capacity == 0 || m_buffer != nullptr);

        m_capacity = o.m_size;
        destructive_move_construct_from(o.m_buffer, o.m_size);
      } else {
        destructive_move_assign_from(o.m_buffer, o.m_size);
      }
      o.reset();
    }
    return *this;
  }

  // ===================== Vector-Interface ===================

  T &operator[](size_type i) {
    assert(i < m_size);
    return m_buffer[i];
  }

  const T &operator[](size_type i) const {
    assert(i < m_size);
    return m_buffer[i];
  }

  void push_back(const T &value) {
    if (m_size == m_capacity) {
      grow(m_capacity == 0 ? 2 : m_capacity * 2);
    }
    if (std::is_trivially_copyable_v<T>) {
      std::memcpy(m_buffer + m_size, &value, sizeof(T));
    } else {
      new (m_buffer + m_size) T(value);
    }
    m_size++;
  }

  void push_back(T &&value) {
    if (m_size == m_capacity) {
      grow(m_capacity == 0 ? 1 : m_capacity * 2);
    }
    new (m_buffer + m_size) T(std::move(value));
    m_size++;
  }

  template <typename... Args> T &emplace_back(Args &&...args) {
    if (m_size == m_capacity) {
      grow(m_capacity == 0 ? 1 : m_capacity * 2);
    }
    new (m_buffer + m_size) T(std::forward<Args>(args)...);
    return m_buffer[m_size++];
  }

  void push_front(const T &value) { insert(begin(), value); }

  void pop_back() {
    assert(m_size != 0);
    m_size--;
    if constexpr (!std::is_trivially_destructible_v<T>) {
      std::destroy_at(m_buffer + m_size);
    }
  }

  void pop_front() {
    assert(m_size != 0);
    if constexpr (std::is_trivially_copyable_v<T>) {
      std::memmove(m_buffer, m_buffer + 1, (m_size - 1) * sizeof(T));
    } else {
      std::move(m_buffer + 1, m_buffer + m_size, m_buffer);
    }
    if (!std::is_trivially_destructible_v<T>) {
      std::destroy_at(m_buffer + m_size - 1);
    }
    --m_size;
  }

  void clear() {
    if constexpr (!std::is_trivially_destructible_v<T>) {
      std::destroy_n(m_buffer, m_size);
    }
    m_size = 0;
  }

  size_type size() const { return m_size; }

  size_type capacity() const { return m_capacity; }

  bool empty() const { return m_size == 0; }

  void reserve(size_type newCapacity) {
    if (newCapacity > m_capacity) {
      grow(newCapacity);
    }
  }

  void resize(size_type newSize, const T &value) {
    if (newSize < m_size) {
      if constexpr (!std::is_trivially_destructible_v<T>) {
        std::destroy(m_buffer + newSize, m_buffer + m_size);
      }
    } else if (newSize > m_size) {
      if (newSize > m_capacity) {
        grow(newSize);
      }
      std::uninitialized_fill(m_buffer + m_size, m_buffer + newSize, value);
    }
    m_size = newSize;
  }

  void resize(size_type newSize)
    requires(std::is_default_constructible_v<T>)
  {
    if (newSize < m_size) {
      if constexpr (!std::is_trivially_destructible_v<T>) {
        std::destroy(m_buffer + newSize, m_buffer + m_size);
      }
    } else if (newSize > m_size) {
      if (newSize > m_capacity) {
        grow(newSize);
      }
      std::uninitialized_value_construct(m_buffer + m_size, m_buffer + newSize);
    }
    m_size = newSize;
  }

  T &back() {
    assert(m_size != 0);
    return m_buffer[m_size - 1];
  }
  const T &back() const {
    assert(m_size != 0);
    return m_buffer[m_size - 1];
  }

  T &front() {
    assert(m_size != 0);
    return m_buffer[0];
  }
  const T &front() const {
    assert(m_size != 0);
    return m_buffer[0];
  }

  // ==================== Special Algorithms ========================
  iterator insert(const_iterator pos, const T &value) {
    if (pos == cend()) {
      push_back(value);
      return end() - 1;
    }
    std::size_t index = pos - begin();
    if (m_size == m_capacity) {
      // NOTE: If we grow we can combine growing and shifting values. This
      // avoids unnecessary move operations.
      std::size_t newCapacity = m_capacity == 0 ? 1 : m_capacity * 2;
      assert(newCapacity > m_capacity);
      T *old = m_buffer;
      size_type oldCapacity = m_capacity;
      m_buffer = ATraits::template allocate<T>(m_allocator, newCapacity);
      assert(m_capacity == 0 || m_buffer != nullptr);
      m_capacity = newCapacity;
      if (old != nullptr) {
        destructive_move_construct_from_helper(
            m_buffer, old, index); // copy first index elements (does not
                                   // include the element at index itself)
        new (m_buffer + index) T(value);
        std::size_t remaining = m_size - index;
        destructive_move_construct_from_helper(
            m_buffer + index + 1, old + index,
            remaining); // copy remaining elements after the index
        ATraits::template deallocate<T>(m_allocator, old, oldCapacity);
      }
    } else {
      if constexpr (std::is_trivially_move_constructible_v<T>) {
        std::memmove(m_buffer + index + 1, m_buffer + index,
                     (m_size - index) * sizeof(T));
      } else {
        std::move_backward(m_buffer + index, m_buffer + m_size,
                           m_buffer + m_size + 1);
      }
      new (m_buffer + index) T(value);
    }
    ++m_size;
    return begin() + index;
  }

  iterator insert(std::size_t i, const T &value) {
    return insert(begin() + i, value);
  }

  // ========================= Range insertion ==================
  template <std::ranges::range R> void append(const R &range) {
    size_type rangeSize = std::ranges::size(range);
    if (rangeSize == 0) {
      return;
    }
    reserve(m_size + rangeSize);
    // Optimized path for contiguous ranges
    if constexpr (std::ranges::contiguous_range<R> &&
                  std::is_trivially_copyable_v<T>) {
      // Direct memcpy for contiguous, trivially copyable range
      std::memcpy(m_buffer + m_size, std::ranges::data(range),
                  rangeSize * sizeof(T));
    } else if constexpr (std::ranges::contiguous_range<R>) {
      // Direct uninitialized copy for contiguous but non-trivially copyable
      // range
      std::uninitialized_copy(std::ranges::begin(range),
                              std::ranges::end(range), m_buffer + m_size);
    } else {
      // Generic fallback for non-contiguous ranges
      std::uninitialized_copy(range.begin(), range.end(), m_buffer + m_size);
    }
    m_size += rangeSize;
  }

  template <std::ranges::range R>
  iterator insert(const_iterator pos, const R &range) {
    const size_type n = std::ranges::size(range);
    if (n == 0)
      return const_cast<iterator>(pos);

    if (pos == cend()) {
      append(range);
      return end() - n;
    }

    const size_type index = pos - begin();

    auto uninit_copy_n = [&](auto first, size_type count, T *dst) {
      if constexpr (std::contiguous_iterator<decltype(first)> &&
                    std::is_trivially_copyable_v<T>) {
        std::memcpy(dst, std::to_address(first), count * sizeof(T));
      } else {
        std::uninitialized_copy_n(first, count, dst);
      }
    };
    if (m_size + n > m_capacity) {
      const size_type newCap =
          std::max(m_capacity ? m_capacity * 2 : size_type(1), m_size + n);

      T *oldBuf = m_buffer;
      const size_type oldCap = m_capacity;

      m_buffer = ATraits::template allocate<T>(m_allocator, newCap);
      m_capacity = newCap;
      assert(m_buffer != nullptr);

      if (oldBuf == nullptr) {
        uninit_copy_n(std::ranges::begin(range), n, m_buffer);
      } else {
        destructive_move_construct_from_helper(m_buffer, oldBuf, index);
        uninit_copy_n(std::ranges::begin(range), n, m_buffer + index);
        destructive_move_construct_from_helper(m_buffer + index + n,
                                               oldBuf + index, m_size - index);

        ATraits::template deallocate<T>(m_allocator, oldBuf, oldCap);
      }
    } else {
      if constexpr (std::is_trivially_move_constructible_v<T>) {
        std::memmove(m_buffer + index + n, m_buffer + index,
                     (m_size - index) * sizeof(T));
      } else {
        std::move_backward(m_buffer + index, m_buffer + m_size,
                           m_buffer + m_size + n);
      }

      uninit_copy_n(std::ranges::begin(range), n, m_buffer + index);
    }

    m_size += n;
    return begin() + index;
  }

  // ================= Stack Interface ==============
  inline void push(const T &value) { push_back(value); }
  inline T &top() { return back(); }
  inline const T &top() const { return back(); }
  inline void pop() { pop_back(); }

  // ================= Set Interface ================
  inline bool contains(const T &value) const {
    const auto e = cend();
    return std::find(cbegin(), e, value) != e;
  }
  inline bool add(const T &value) {
    if (contains(value)) {
      return false;
    } else {
      push_back(value);
      return true;
    }
  }
  // NOTE: The order of elements is undefined after removing an element.
  inline bool remove(const T &value) {
    const auto e = end();
    auto it = std::find(begin(), e, value);
    if (it == e) {
      return false;
    } else {
      --m_size;
      *it = std::move(m_buffer[m_size]);
      std::destroy_at(m_buffer + m_size);
      return true;
    }
  }
  // ================= FIFO-Queue Interface ================
  void enqueue(const T &value) { 
    push_back(value);
  }

  const T &peek() const { return front(); }

  T dequeue() {
    T value = std::move(*m_buffer);
    pop_front();
    return value;
  }

  // ================= Range Interface ===============
  inline iterator begin() { return m_buffer; }
  inline const_iterator begin() const { return m_buffer; }
  inline iterator end() { return m_buffer + m_size; }
  inline const_iterator end() const { return m_buffer + m_size; }
  // Optional reverse iterator interface
  inline reverse_iterator rbegin() { return reverse_iterator(end()); }
  inline reverse_iterator rend() { return reverse_iterator(begin()); }
  inline const_reverse_iterator rbegin() const {
    return const_reverse_iterator(end());
  }
  inline const_reverse_iterator rend() const {
    return const_reverse_iterator(begin());
  }
  // Optional constant interface.
  inline const_iterator cbegin() const { return m_buffer; }
  inline const_iterator cend() const { return m_buffer + m_size; }
  inline const_reverse_iterator crbegin() const {
    return const_reverse_iterator(end());
  }
  inline const_reverse_iterator crend() const {
    return const_reverse_iterator(begin());
  }

private:
  void grow(size_type newCapacity) {
    assert(newCapacity > m_capacity);
    T *old = m_buffer;
    size_type oldCapacity = m_capacity;
    m_buffer = ATraits::template allocate<T>(m_allocator, newCapacity);
    assert(m_capacity == 0 || m_buffer != nullptr);
    m_capacity = newCapacity;
    if (old != nullptr) {
      destructive_move_construct_from(old, m_size);
      ATraits::template deallocate<T>(m_allocator, old, oldCapacity);
    }
  }

  void reset() {
    std::destroy(m_buffer, m_buffer + m_size);
    release();
    m_buffer = nullptr;
    m_capacity = 0;
    m_size = 0;
  }

  void copy_construct_from(T *source, size_type size) {
    assert(m_capacity >= size);
    if constexpr (std::is_trivially_copyable_v<T>) {
      // Trivial objects can be copied with memcpy, which is most likely
      // faster!
      std::memcpy(m_buffer, source, size * sizeof(T));
    } else {
      // For non trivial objects we have to call the copy constructor.
      // Uninitalized copy implies that m_buffer is raw memory and does not
      // contain any constructed objects (No need to call destructor).
      std::uninitialized_copy(source, source + size, m_buffer);
    }
    m_size = size;
  }

  void copy_assign_from(T *source, size_type size) {
    assert(m_capacity >= size);
    if constexpr (std::is_trivially_copyable_v<T>) {
      std::memcpy(m_buffer, source, size * sizeof(T));
    } else {
      if (size <= m_size) {
        // Copy elements (Calls copy assignment operator i.e. destructs
        // previous elements)
        std::copy(source, source + size, m_buffer);
        // Destruct remaining objects.
        std::destroy(m_buffer + size, m_buffer + m_size);
      } else {
        std::copy(source, source + m_size, m_buffer);
        std::uninitialized_copy(source + m_size, source + size, m_buffer);
      }
    }
    m_size = size;
  }
  static void destructive_move_construct_from_helper(T *buffer, T *source,
                                                     size_type size) {
    if constexpr (std::is_trivially_move_constructible_v<T>) {
      // Trivial objects can be copied with memcpy, which is most likely
      // faster!
      std::memcpy(buffer, source, size * sizeof(T));
    } else {
      // For non trivial objects we have to call the copy constructor.
      // Uninitalized copy implies that m_buffer is raw memory and does not
      // contain any constructed objects (No need to call destructor).
      std::uninitialized_move(source, source + size, buffer);
    }
    if constexpr (!strobe::is_trivially_destructible_after_move_v<T>) {
      std::destroy(source, source + size);
    }
  }

  void destructive_move_construct_from(T *source, size_type size) {
    destructive_move_construct_from_helper(m_buffer, source, size);
    m_size = size;
  }

  void destructive_move_assign_from(T *source, size_type size) {
    assert(m_capacity >= size);
    if constexpr (std::is_trivially_move_assignable_v<T>) {
      std::memcpy(m_buffer, source, size * sizeof(T));
    } else {
      if (size <= m_size) {
        // Copy elements (Calls copy assignment operator i.e. destructs
        // previous elements)
        std::move(source, source + size, m_buffer);
        // Destruct remaining objects.
        std::destroy(m_buffer + size, m_buffer + m_size);
      } else {
        std::move(source, source + m_size, m_buffer);
        std::uninitialized_move(source + m_size, source + size, m_buffer);
      }
      if constexpr (!strobe::is_trivially_destructible_after_move_v<T>) {
        std::destroy(source, source + size);
      }
    }
    m_size = size;
  }

  void release() {
    if (m_buffer != nullptr) {
      ATraits::template deallocate<T>(m_allocator, m_buffer, m_capacity);
      m_buffer = nullptr;
      m_capacity = 0;
      m_size = 0;
    }
  }

  size_type m_capacity;
  size_type m_size;
  T *m_buffer;
  [[no_unique_address]] A m_allocator;
};

static_assert(strobe::Container<Vector<int>>);
static_assert(strobe::RandomAccessContainer<Vector<int>>);
static_assert(strobe::StackLikeContainer<Vector<int>>);
static_assert(strobe::SetLikeContainer<Vector<int>>);
static_assert(strobe::ContainerSupportsInsertion<Vector<int>>);
static_assert(strobe::ContainerSupportsRangeInsertion<Vector<int>, Vector<int>>);

static_assert(strobe::QueueLikeContainer<Vector<int>>);
static_assert(std::ranges::range<Vector<int>>);
static_assert(std::ranges::forward_range<Vector<int>>);
static_assert(std::ranges::bidirectional_range<Vector<int>>);
static_assert(std::ranges::random_access_range<Vector<int>>);
static_assert(std::ranges::contiguous_range<Vector<int>>);

} // namespace strobe
