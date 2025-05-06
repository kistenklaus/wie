#pragma once

#include <concepts>
#include <cstddef>
#include <utility>
namespace strobe {

template <typename A>
concept Allocator =
    requires(A a, std::size_t size, std::size_t align, void* ptr) {
      { a.allocate(size, align) } -> std::same_as<void*>;
      { a.deallocate(ptr, size, align) };
    };

template <typename A>
concept ReAllocator =
    Allocator<A> && requires(A a, void* ptr, std::size_t oldSize,
                             std::size_t newSize, std::size_t align) {
      { a.reallocate(ptr, oldSize, newSize, align) } -> std::same_as<void*>;
    };

template <typename A>
concept OverAllocator =
    Allocator<A> && requires(A a, std::size_t size, std::size_t align) {
      {
        a.allocate_at_least(size, align)
      } -> std::same_as<std::pair<void*, std::size_t>>;
    };

template <typename A>
concept OwningAllocator = Allocator<A> && requires(const A a, void* ptr) {
  { a.owns(ptr) } -> std::same_as<bool>;
};

template <typename A>
concept SizeIndepdententAllocator =
    Allocator<A> && requires(A a, void* ptr) {
      { a.deallocate(ptr) };
    };

template <Allocator A>
struct AllocatorTraits {
  static inline void* allocate(A& a, std::size_t size, std::size_t align) {
    return a.allocate(size, align);
  }
  static inline void deallocate(A& a, void* ptr, std::size_t size,
                                std::size_t align) {
    a.deallocate(ptr, size, align);
  }

  static inline void size_independent_deallocate(A& a, void* ptr)
    requires SizeIndepdententAllocator<A>
  {
    return a.deallocate(ptr);
  }

  template <typename T>
  static inline T* allocate(A& a, std::size_t n = 1) {
    return reinterpret_cast<T*>(allocate(a, n * sizeof(T), alignof(T)));
  }

  template <typename T>
  static inline void deallocate(A& a, T* ptr, std::size_t n = 1) {
    deallocate(a, ptr, n * sizeof(T), alignof(T));
  }

  static std::pair<void*, std::size_t> allocate_at_least(...);
  static void* reallocate(...);
  static bool owns(A& a, void* ptr) requires OwningAllocator<A> {
    return a.owns(ptr);
  }
};

}  // namespace strobe
