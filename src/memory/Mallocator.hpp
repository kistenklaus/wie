#pragma once

#include <cstdlib>

namespace strobe {

class Mallocator {
 public:
  void* allocate(std::size_t size, std::size_t align) {
    return std::malloc(size);  // You could use aligned_alloc if necessary
  }

  void deallocate(void* ptr, std::size_t size, std::size_t) {
    std::free(ptr);
  }

  void deallocate(void* ptr) {
    std::free(ptr);
  }
};

}  // namespace strobe
