#include "vec.h"
#include <gtest/gtest.h>
#include <memory>
#include <ranges>

struct A {
  A() : x(0){
  }
  int x;
};

struct B : public A {
  B() : A(), y(0) {
  }
  int y;
};


TEST(Vec, BasicAssertions) {
  using namespace wie;




  B b;
  A a = b;

  Vec<int> v1{};
  v1.push_back(1);
  v1.push_back(2);
  
  static_assert(std::ranges::contiguous_range<Vec<int>>);

}


