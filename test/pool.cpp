#include "counting_allocator.h"
#include "pool.h"
#include <gtest/gtest.h>

TEST(LinkedPoolBucket, BasicAssertions) {
  wie::FreelistPool<1> pool;
  std::vector<void *> mem;
  for (size_t i = 0; i < 10000000; ++i) {
    mem.push_back(pool.allocate());
  }

  for (auto p : mem) {
    pool.deallocate(p);
  }
}

TEST(LinkedPoolBucket, ParentAllocator) {
  constexpr size_t ELEM_SIZE = 32;
  constexpr size_t CHUNK_SIZE = std::max((size_t)1, 4096 / ELEM_SIZE * 8);

  using CountingAllocator = wie::CountingAllocator<std::allocator<
      wie::internal::LinkedPoolBucketChunk<ELEM_SIZE, CHUNK_SIZE>>>;

  CountingAllocator countingAllocator{};
  {

    wie::FreelistPool<ELEM_SIZE, CHUNK_SIZE, CountingAllocator> pool{
        countingAllocator};
  }

  ASSERT_EQ(countingAllocator.allocCount(), 1);
  ASSERT_EQ(countingAllocator.deallocCount(), 1);
}
