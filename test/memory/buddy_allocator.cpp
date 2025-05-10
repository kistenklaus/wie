#include "memory/BuddyResource.hpp"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <gtest/gtest.h>
#include <random>

using namespace strobe;

TEST(BuddyAllocator, simple_pool_allocations) {
  static constexpr std::size_t COUNT = 32;
  strobe::BuddyResource<sizeof(std::uint32_t) * COUNT, sizeof(std::uint32_t)>
      resource;

  std::vector<std::uint32_t *> allocations(COUNT);
  for (std::size_t i = 0; i < COUNT; ++i) {
    allocations[i] = reinterpret_cast<std::uint32_t *>(
        resource.allocate(sizeof(std::uint32_t), alignof(std::uint32_t)));
    EXPECT_NE(allocations[i], nullptr) << "Allocation " << i << " failed";
    EXPECT_TRUE(resource.owns(allocations[i]))
        << "Allocation " << i << " is not owned by resource";
  }

  for (std::uint32_t i = 0; i < COUNT; ++i) {
    *allocations[i] = i;
  }

  for (std::uint32_t i = 0; i < COUNT; ++i) {
    EXPECT_EQ(*allocations[i], i) << "Allocation " << i << " is overlapping";
  }
}

TEST(BuddyAllocator, simple_allocations) {
  static constexpr std::size_t Capacity = 256;
  strobe::BuddyResource<Capacity, sizeof(std::uint32_t)> resource;

  std::vector<std::size_t> sizes = {32, 16, 4, 8, 64, 8, 8, 16};
  std::vector<std::uint32_t *> pointers(sizes.size());

  for (std::size_t i = 0; i < sizes.size(); ++i) {
    pointers[i] = reinterpret_cast<std::uint32_t *>(
        resource.allocate(sizes[i], alignof(std::uint32_t)));
    for (std::size_t j = 0; j < sizes[i] / sizeof(std::uint32_t); ++j) {
      pointers[i][j] = i;
    }
  }

  for (std::size_t i = 0; i < sizes.size(); ++i) {
    for (std::size_t j = 0; j < sizes[i] / sizeof(std::uint32_t); ++j) {
      EXPECT_EQ(pointers[i][j], i);
    }
  }
}

TEST(BuddyAllocator, overallocation) {
  static constexpr std::size_t COUNT = 32;
  strobe::BuddyResource<sizeof(std::uint32_t) * COUNT, sizeof(std::uint32_t)>
      resource;

  std::vector<std::uint32_t *> allocations(COUNT);
  for (std::size_t i = 0; i < COUNT; ++i) {
    allocations[i] = reinterpret_cast<std::uint32_t *>(
        resource.allocate(sizeof(std::uint32_t), alignof(std::uint32_t)));
    EXPECT_NE(allocations[i], nullptr) << "Allocation " << i << " failed";
    EXPECT_TRUE(resource.owns(allocations[i]))
        << "Allocation " << i << " is not owned by resource";
  }

  for (std::uint32_t i = 0; i < COUNT; ++i) {
    *allocations[i] = i;
  }

  void *p = resource.allocate(sizeof(std::uint32_t), alignof(std::uint32_t));
  EXPECT_EQ(p, nullptr) << "Overallocations should return nullptr";

  for (std::uint32_t i = 0; i < COUNT; ++i) {
    EXPECT_EQ(*allocations[i], i) << "Allocation " << i << " is overlapping";
  }
}

TEST(BuddyAllocator, simple_pool_alloc_and_dealloc) {
  static constexpr std::size_t COUNT = 32;
  strobe::BuddyResource<sizeof(std::uint32_t) * COUNT, sizeof(std::uint32_t)>
      resource;

  std::vector<std::uint32_t *> allocations(COUNT);
  for (std::size_t i = 0; i < COUNT; ++i) {
    allocations[i] = reinterpret_cast<std::uint32_t *>(
        resource.allocate(sizeof(std::uint32_t), alignof(std::uint32_t)));
    EXPECT_NE(allocations[i], nullptr) << "Allocation " << i << " failed";
    EXPECT_TRUE(resource.owns(allocations[i]))
        << "Allocation " << i << " is not owned by resource";
  }

  for (std::uint32_t i = 0; i < COUNT; ++i) {
    *allocations[i] = i;
  }

  for (std::uint32_t i = 0; i < COUNT; ++i) {
    EXPECT_EQ(*allocations[i], i) << "Allocation " << i << " is overlapping";
  }

  for (int i = COUNT - 1; i >= 0; --i) {
    resource.deallocate(allocations[i], sizeof(std::uint32_t),
                        alignof(std::uint32_t));
  }
}

TEST(BuddyAllocator, realloc_returns_same_address) {
  static constexpr std::size_t COUNT = 32;
  strobe::BuddyResource<sizeof(std::uint32_t) * COUNT, sizeof(std::uint32_t)>
      resource;

  void *a1 = resource.allocate(sizeof(std::uint32_t), alignof(std::uint32_t));
  EXPECT_NE(a1, nullptr);
  EXPECT_TRUE(resource.owns(a1)) << "Allocation a1 is not owned by resource";
  resource.deallocate(a1, sizeof(std::uint32_t), alignof(std::uint32_t));
  void *a2 = resource.allocate(sizeof(std::uint32_t), alignof(std::uint32_t));
  EXPECT_EQ(a1, a2);
}

TEST(BuddyAllocator, simple_alloc_dealloc) {

  strobe::BuddyResource<32 * 4, 4> resource;

  std::vector<std::size_t> sizes = {4, 8, 16, 32};
  std::vector<void *> pointers(sizes.size());

  auto test = [&](std::size_t i) {
    if (pointers[i]) {
      std::uint32_t j = *reinterpret_cast<std::uint32_t *>(pointers[i]);
      EXPECT_EQ(j, i) << "Allocation " << i << " is overlapping with " << j;
      resource.deallocate(pointers[i], sizes[i], alignof(std::uint32_t));
      pointers[i] = nullptr;
    } else {
      pointers[i] = resource.allocate(sizes[i], alignof(std::uint32_t));
      *reinterpret_cast<std::uint32_t *>(pointers[i]) = i;
    }
  };

  test(3);
  test(0);
  test(1);
  test(0);
  test(2);
  test(1);
  test(3);
  test(2);
}

TEST(BuddyAllocator, random_alloc_dealloc) {
  constexpr std::size_t Capacity = 1ull << 20;
  strobe::BuddyResource<Capacity, sizeof(std::uint32_t)> resource;

  std::array allocationSizeOptions = {
      sizeof(std::uint32_t),      sizeof(std::uint32_t) * 2,
      sizeof(std::uint32_t) * 4,  sizeof(std::uint32_t) * 8,
      sizeof(std::uint32_t) * 16, sizeof(std::uint32_t) * 32};

  std::size_t count =
      (Capacity / 2) / (*std::ranges::max_element(allocationSizeOptions));

  std::vector<void *> allocations(count);
  std::vector<std::uint32_t> values(count);
  std::vector<std::size_t> allocationSize(count);
  std::vector<std::size_t> shflIdx(2 * count);

  std::mt19937 rng;
  std::uniform_int_distribution<std::size_t> sizeDist{
      0, allocationSizeOptions.size() - 1};
  for (std::size_t i = 0; i < count; ++i) {
    allocations[i] = nullptr;
    allocationSize[i] = allocationSizeOptions[sizeDist(rng)];
    shflIdx[2 * i] = i;
    shflIdx[2 * i + 1] = i;
  }

  for (std::uint32_t it = 0; it < 4; ++it) {
    std::ranges::shuffle(shflIdx, rng);

    for (auto idx : shflIdx) {
      if (allocations[idx] == nullptr) {
        allocations[idx] =
            resource.allocate(allocationSize[idx], alignof(std::uint32_t));
        *reinterpret_cast<std::uint32_t *>(allocations[idx]) = idx;
        // std::println("alloc {}   size={}    0x{:X}", idx,
        // allocationSize[idx],
        //             reinterpret_cast<std::size_t>(allocations[idx]));
        ASSERT_NE(allocations[idx], nullptr);
      } else {
        // std::println("dealloc {}   size={}    0x{:X}", idx,
        // allocationSize[idx],
        //             reinterpret_cast<std::size_t>(allocations[idx]));
        ASSERT_EQ(*reinterpret_cast<std::uint32_t *>(allocations[idx]), idx);
        resource.deallocate(allocations[idx], allocationSize[idx],
                            alignof(std::uint32_t));

        allocations[idx] = nullptr;
      }
    }
  }
}
