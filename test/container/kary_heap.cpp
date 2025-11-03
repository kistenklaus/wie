#include "container/kary_heap.hpp"
#include <gtest/gtest.h>


TEST(container_kary_heap, simple) {
  strobe::KAryHeap<int, 5> heap;


  heap.push(1);
  heap.push(3);
  heap.push(2);

  EXPECT_EQ(heap.top(), 1);
  heap.pop();
  EXPECT_EQ(heap.top(), 2);
  heap.pop();
  EXPECT_EQ(heap.top(), 3);

  EXPECT_EQ(heap.size(), 1);
  heap.pop();
  EXPECT_TRUE(heap.empty());
}
