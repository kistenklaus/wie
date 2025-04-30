#include <gtest/gtest.h>
#include <memory/Mallocator.hpp>

TEST(Mallocator, trivial) {

  strobe::Mallocator mallocator;

  void* p = mallocator.allocate(sizeof(std::uint64_t), alignof(std::uint64_t));

  ASSERT_TRUE(reinterpret_cast<std::intptr_t>(p) % (alignof(std::uint64_t)) == 0);

  mallocator.deallocate(p);

}


