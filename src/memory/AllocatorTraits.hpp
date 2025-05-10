#pragma once

#include <concepts>
#include <cstddef>
#include <utility>
namespace strobe {

template <typename A>
concept Allocator =
    requires(A a, std::size_t size, std::size_t align, void *ptr) {
      { a.allocate(size, align) } -> std::same_as<void *>;
      { a.deallocate(ptr, size, align) };
    };

template <typename A>
concept ReAllocator =
    Allocator<A> && requires(A a, void *ptr, std::size_t oldSize,
                             std::size_t newSize, std::size_t align) {
      { a.reallocate(ptr, oldSize, newSize, align) } -> std::same_as<void *>;
    };

template <typename A>
concept OverAllocator =
    Allocator<A> && requires(A a, std::size_t size, std::size_t align) {
      {
        a.allocate_at_least(size, align)
      } -> std::same_as<std::pair<void *, std::size_t>>;
    };

template <typename A>
concept OwningAllocator = Allocator<A> && requires(const A a, void *ptr) {
  { a.owns(ptr) } -> std::same_as<bool>;
};

template <typename A>
concept SizeIndepdententAllocator = Allocator<A> && requires(A a, void *ptr) {
  { a.deallocate(ptr) };
};

template <typename A>
concept ComparibleAllocator = Allocator<A> && requires(A a) {
  { a == a } -> std::same_as<bool>;
  { a != a } -> std::same_as<bool>;
};

template <Allocator A> struct AllocatorTraits {

  template <typename T> using pointer = T *;
  template <typename T> using const_pointer = const T *;

  static inline void *allocate(A &a, std::size_t size, std::size_t align) {
    return a.allocate(size, align);
  }
  static inline void deallocate(A &a, void *ptr, std::size_t size,
                                std::size_t align) {
    a.deallocate(ptr, size, align);
  }

  static inline void size_independent_deallocate(A &a, void *ptr)
    requires SizeIndepdententAllocator<A>
  {
    return a.deallocate(ptr);
  }

  template <typename T> static inline T *allocate(A &a, std::size_t n = 1) {
    return reinterpret_cast<T *>(allocate(a, n * sizeof(T), alignof(T)));
  }

  template <typename T>
  static inline void deallocate(A &a, T *ptr, std::size_t n = 1) {
    deallocate(a, ptr, n * sizeof(T), alignof(T));
  }

  static std::pair<void *, std::size_t> allocate_at_least(...);
  static void *reallocate(...);
  static bool owns(A &a, void *ptr)
    requires OwningAllocator<A>
  {
    return a.owns(ptr);
  }

  // Copy semantics
  static inline A select_on_container_copy_construction(const A &a) {
    return a;
  }

private:
  template <class U> static constexpr bool pocca_value() noexcept {
    if constexpr (requires {
                    {
                      U::propagate_on_container_copy_assignment
                    } -> std::convertible_to<bool>;
                  }) {
      return static_cast<bool>(U::propagate_on_container_copy_assignment);
    } else {
      return false;
    }
  }
  template <class U> static inline constexpr bool pomca_value() noexcept {
    if constexpr (requires {
                    {
                      U::propagate_on_container_move_assignment
                    } -> std::convertible_to<bool>;
                  })
      return static_cast<bool>(U::propagate_on_container_move_assignment);
    else
      return false;
  }

  template <class U>
  static inline constexpr bool is_always_equal_value() noexcept {
    if constexpr (requires {
                    { U::is_always_equal } -> std::convertible_to<bool>;
                  })
      return static_cast<bool>(U::is_always_equal);
    else
      return false;
  }

public:
  static constexpr bool propagate_on_container_copy_assignment =
      pocca_value<A>();

  static constexpr bool propagate_on_container_move_assignment = pomca_value<A>;

  static constexpr bool is_always_equal =
      !ComparibleAllocator<A> && is_always_equal_value<A>();
};

template <class L, class R>
constexpr bool alloc_equals(const L &lhs, const R &rhs) noexcept {
  /* ---------- same type ------------------------------------------------- */
  if constexpr (std::same_as<L, R>) {
    if constexpr (ComparibleAllocator<L>) {
      return lhs == rhs; // (1)
    } else {
      return AllocatorTraits<L>::is_always_equal; // still fine
    }
  }
  /* ---------- different types ------------------------------------------ */
  else {
    constexpr bool lhs_always = AllocatorTraits<L>::is_always_equal;
    constexpr bool rhs_always = AllocatorTraits<R>::is_always_equal;

    return lhs_always && rhs_always; // (2)  otherwise false
  }
}

} // namespace strobe
