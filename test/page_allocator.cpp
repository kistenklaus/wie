#include "memory/pages.hpp"
#include <gtest/gtest.h>
#include <memory/PageAllocator.hpp>

TEST(PageAllocator, small) {

  strobe::PageAllocator allocator;

  void *p = allocator.allocate(10 * sizeof(std::uint64_t), alignof(std::uint64_t));

  std::uint64_t *x = reinterpret_cast<std::uint64_t *>(p);
  [[maybe_unused]] volatile std::uint64_t t;
  t = x[9];
  *x = 10;
  t = *x;

  ASSERT_TRUE((reinterpret_cast<std::intptr_t>(p) % strobe::page_size()) == 0);

  allocator.deallocate(p, 10 * sizeof(std::uint64_t), alignof(std::uint64_t));
}

TEST(PageAllocator, large) {

  strobe::PageAllocator allocator;

  void *p = allocator.allocate(1 << 29, alignof(std::uint64_t));

  std::uint64_t *x = reinterpret_cast<std::uint64_t *>(p);
  *x = 10;

  ASSERT_TRUE((reinterpret_cast<std::intptr_t>(p) % strobe::page_size()) == 0);

  allocator.deallocate(p, 1 << 29, alignof(std::uint64_t));
}
