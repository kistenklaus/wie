#pragma once

#include "memory/AllocatorTraits.hpp"
namespace strobe {

template <Allocator Resource> class AllocatorReference {
public:
  AllocatorReference(Resource *resource) : m_resource(resource) {}

  void *allocate(std::size_t size, std::size_t align) {
    return Traits::allocate(*m_resource, size, align);
  }

  void deallocate(void *ptr, std::size_t size, std::size_t align) {
    return Traits::free(*m_resource, ptr, size, align);
  }

  void deallocate(void *ptr)
    requires(SizeIndepdententAllocator<Resource>)
  {
    return Traits::size_independent_deallocate(*m_resource, ptr);
  }

  bool owns(void *ptr)
    requires OwningAllocator<Resource>
  {
    return Traits::owns(*m_resource, ptr);
  }

private:
  using Traits = AllocatorTraits<Resource>;
  Resource *m_resource;
};

} // namespace strobe::memory
