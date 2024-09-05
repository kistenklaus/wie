#include "counting_allocator.h"
#include <gtest/gtest.h>
#include <memory>

TEST(CountingAllocator, BasicAssertions) {
  using CAL = wie::CountingAllocator<std::allocator<int>>;;
  CAL cal;
  ASSERT_EQ(cal.allocCount(), 0);
  ASSERT_EQ(cal.deallocCount(), 0);
  auto p = cal.allocate(1);
  ASSERT_EQ(cal.allocCount(), 1);
  ASSERT_EQ(cal.deallocCount(), 0);
  cal.deallocate(p, 1);
  ASSERT_EQ(cal.allocCount(), 1);
  ASSERT_EQ(cal.deallocCount(), 1);
}

TEST(CountingAllocator, CopyConstructor) {
  using CAL = wie::CountingAllocator<std::allocator<int>>;;
  CAL cal;
  CAL cal2 = CAL(cal);

  cal2.allocate(1);
  ASSERT_EQ(cal.allocCount(), 1);
  ASSERT_EQ(cal2.allocCount(), 1);
}

TEST(CountingAllocator, CopyConstructorAllocTraits) {
  using CAL = wie::CountingAllocator<std::allocator<int>>;;
  CAL cal;
  using AllocTraits = std::allocator_traits<CAL>;
  CAL cal2 = AllocTraits::select_on_container_copy_construction(cal);

  auto p = cal2.allocate(1);
  cal.deallocate(p, 1);
  ASSERT_EQ(cal.allocCount(), 1);
  ASSERT_EQ(cal2.allocCount(), 1);
  ASSERT_EQ(cal.deallocCount(), 1);
  ASSERT_EQ(cal2.deallocCount(), 1);
}

TEST(CountingAllocator, CopyAssignment) {
  using CAL = wie::CountingAllocator<std::allocator<int>>;;
  CAL cal;
  CAL cal2 = cal;

  cal2.allocate(1);
  ASSERT_EQ(cal.allocCount(), 1);
  ASSERT_EQ(cal2.allocCount(), 1);
}

TEST(CountingAllocator, CopyAssignmentAllocTraits) {
  using CAL = wie::CountingAllocator<std::allocator<int>>;;
  CAL cal;
  using AllocTraits = std::allocator_traits<CAL>;
  CAL calx = AllocTraits::select_on_container_copy_construction(cal);
  CAL cal2 = calx;

  auto p = cal2.allocate(1);
  cal.deallocate(p, 1);
  ASSERT_EQ(cal.allocCount(), 1);
  ASSERT_EQ(cal2.allocCount(), 1);
  ASSERT_EQ(cal.deallocCount(), 1);
  ASSERT_EQ(cal2.deallocCount(), 1);
}

TEST(CountingAllocator, MoveAssignment) {
  using CAL = wie::CountingAllocator<std::allocator<int>>;;
  CAL cal;
  CAL cal2 = cal;
  CAL cal3 = std::move(cal);


  auto p = cal2.allocate(1);
  cal3.deallocate(p, 1);
  ASSERT_EQ(cal3.allocCount(), 1);
  ASSERT_EQ(cal2.allocCount(), 1);
  ASSERT_EQ(cal3.deallocCount(), 1);
  ASSERT_EQ(cal2.deallocCount(), 1);
}

TEST(CountingAllocator, MoveConstructor) {
  using CAL = wie::CountingAllocator<std::allocator<int>>;;
  CAL cal;
  CAL cal2 = cal;
  CAL cal3 = CAL(std::move(cal));


  auto p = cal2.allocate(1);
  cal3.deallocate(p, 1);
  ASSERT_EQ(cal3.allocCount(), 1);
  ASSERT_EQ(cal2.allocCount(), 1);
  ASSERT_EQ(cal3.deallocCount(), 1);
  ASSERT_EQ(cal2.deallocCount(), 1);
}

TEST(CountingAllocator, Equality) {
  using CAL = wie::CountingAllocator<std::allocator<int>>;;
  CAL cal;
  CAL cal2 = cal;

  CAL calx;

  ASSERT_EQ(cal, cal2);
  ASSERT_NE(cal, calx);
  ASSERT_NE(cal2, calx);
}
