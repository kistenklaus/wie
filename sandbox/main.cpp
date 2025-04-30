#include "memory/BuddyAllocator.hpp"
#include "memory/Mallocator.hpp"
#include <print>

int main() {

  constexpr std::size_t Capacity = 1ull << 10;
  strobe::BuddyResource<Capacity, sizeof(std::uint32_t)> resource;
}
