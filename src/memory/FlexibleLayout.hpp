#pragma once

#include <cstddef>
#include <utility>

#include "memory/PolyAllocator.hpp"
#include "memory/align.hpp"
namespace strobe {

template <typename T> class TailView {
public:
  T *get(void *p) const noexcept {
    return reinterpret_cast<T *>(reinterpret_cast<std::byte *>(p) + m_offset);
  }

  void set(void *p, T value) const noexcept { *get(p) = std::move(value); }

private:
  explicit TailView(std::size_t offset) : m_offset(offset) {}
  friend class TailAllocator;
  std::size_t m_offset;
};

class TailAllocator;

namespace detail {

struct any {
    template<typename T>
    operator T(); // convertible to anything
};

template<typename T, typename = void>
struct is_flexible_layout : std::false_type {};

template<typename T>
struct is_flexible_layout<T, std::void_t<
    decltype(T::make(std::declval<TailAllocator&>())),
    decltype(T::make(std::declval<TailAllocator&>(), any{})),
    decltype(T::make(std::declval<TailAllocator&>(), any{}, any{})),
    decltype(T::make(std::declval<TailAllocator&>(), any{}, any{}, any{})),
    decltype(T::make(std::declval<TailAllocator&>(), any{}, any{}, any{}, any{}))
>> : std::true_type {};

} // namespace detail

template <typename T>
concept FlexibleLayout = detail::is_flexible_layout<T>::value;

class TailAllocator {
public:
  // Internal use only!

  TailAllocator(PolyAllocator alloc, std::size_t headerSize,
                std::size_t headerAlignment)
      : m_alloc(std::move(alloc)), m_headerSize(headerSize),
        m_headerAlignment(headerAlignment), m_offset(m_headerSize),
        m_ptr(nullptr) {}

  TailAllocator(const TailAllocator &) = delete;
  TailAllocator &operator=(const TailAllocator &) = delete;
  TailAllocator(TailAllocator &&) = delete;
  TailAllocator &operator=(TailAllocator &&) = delete;

  template <typename T>
  TailView<T> reserve(std::size_t n = 1, std::size_t alignment = alignof(T)) {
    assert(m_ptr == nullptr);
    std::size_t aligned = strobe::align_up(m_offset, alignment);
    m_offset = aligned + n * sizeof(T);
    return TailView<T>{aligned};
  }

  void *allocate() {
    m_ptr = m_alloc.allocate(m_offset, m_headerAlignment);
    assert(m_ptr != nullptr);
    return m_ptr;
  }

  void *header() const {
    assert(m_ptr != nullptr);
    return m_ptr;
  }

  void *get() const {
    assert(m_ptr != nullptr);
    return m_ptr;
  }

  std::size_t totalSize() const {
    assert(m_ptr != nullptr);
    return m_offset;
  }

private:
  PolyAllocator m_alloc;
  const std::size_t m_headerSize;
  const std::size_t m_headerAlignment;
  std::size_t m_offset;
  void *m_ptr;
};

}; // namespace strobe
