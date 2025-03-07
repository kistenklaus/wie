#include <algorithm>
#include <gtest/gtest.h>
#include <iterator>
#include <ranges>
#include "singly_linked_list.h"

TEST(SinglyLinkedList, BasicAssertions) {
  using namespace wie;

  SinglyLinkedList<int> l1;
  l1.push_front(1);

  SinglyLinkedList<int> l2;
  l2.push_front(2);

  std::vector v = {1,2,3};

  l1.insert_range_after(l1.begin(), v);

  static_assert(std::forward_iterator<decltype(l1.begin())>);
  /*  */
  static_assert(std::ranges::forward_range<SinglyLinkedList<int>>);
  


  

}
