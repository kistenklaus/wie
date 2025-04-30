#pragma once

#include <cstddef>

#include "memory/AllocatorTraits.hpp"
#include "memory/Mallocator.hpp"
namespace strobe {

namespace detail {

class MemoryResource {
 public:
  virtual void* allocate(std::size_t size, std::size_t align) = 0;
  virtual void deallocate(void* ptr, std::size_t size, std::size_t align) = 0;
};

template <Allocator A>
class MemoryResourceAllocatorAdapter : public MemoryResource {
  using Traits = AllocatorTraits<A>;

 public:
  explicit MemoryResourceAllocatorAdapter(A* upstream) : m_upstream(upstream) {}

  void* allocate(std::size_t size, std::size_t align) final override {
    return Traits::allocate(*m_upstream, size, align);
  }
  void deallocate(void* ptr, std::size_t size,
                  std::size_t align) final override {
    return Traits::deallocate(*m_upstream, ptr, size, align);
  }

 private:
  A* m_upstream;
};

}  // namespace detail

class PolyAllocator {
 public:
  template <Allocator A>
  PolyAllocator(A* upstream) {
    using Adapter = detail::MemoryResourceAllocatorAdapter<A>;
    static_assert(sizeof(Adapter) <= sizeof(m_storage));
    new (m_storage) Adapter(upstream);
  }

  void* allocate(std::size_t size, std::size_t align) {
    detail::MemoryResource* resource =
        reinterpret_cast<detail::MemoryResource*>(this->m_storage);
    return resource->allocate(size, align);
  }

  void deallocate(void* ptr, std::size_t size, std::size_t align) {
    detail::MemoryResource* resource =
        reinterpret_cast<detail::MemoryResource*>(this->m_storage);
    resource->deallocate(ptr, size, align);
  }

 private:
  alignas(detail::MemoryResourceAllocatorAdapter<Mallocator>) std::byte
      m_storage[sizeof(detail::MemoryResourceAllocatorAdapter<Mallocator>)];
};

}  // namespace strobe
