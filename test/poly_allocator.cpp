#include "memory/Mallocator.hpp"
#include "memory/PolyAllocator.hpp"
#include <gtest/gtest.h>

TEST(PolyAllocator, simple_mallocator) {

  strobe::Mallocator mallocator;
  strobe::PolyAllocator poly{&mallocator};


  void* p = poly.allocate(sizeof(std::uint64_t), alignof(std::uint64_t));

  std::uint64_t* x = reinterpret_cast<std::uint64_t*>(p);
  std::uint64_t t = *x;
  *x = 10;
  t = *x;

  ASSERT_TRUE(reinterpret_cast<std::intptr_t>(p) % (alignof(std::uint64_t)) == 0);

  poly.deallocate(p, sizeof(std::uint64_t), alignof(std::uint64_t));
}


