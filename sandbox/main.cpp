#include "memory/BuddyResource.hpp"
#include "memory/Mallocator.hpp"
#include <print>

int main() {

  constexpr std::size_t Capacity = 64;
  strobe::BuddyResource<Capacity, 4> resource;

  void* p = resource.allocate(4,4);
  void* q = resource.allocate(8,4);

  resource.deallocate(q, 8,8);

  return 0;
}
