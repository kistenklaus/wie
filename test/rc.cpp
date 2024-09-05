#include "rc.h"
#include <gtest/gtest.h>
#include <vector>

TEST(Rc, BasicAssertions) {
  wie::Rc<int> x = wie::Rc<int>::make(1);

  wie::Rc<int> y = wie::Rc<int>(x);

  wie::Rc<int> z = y;

  wie::Rc<int> w = std::move(z);

  ASSERT_EQ(*x, *y);
  ASSERT_EQ(*x, *z);
  ASSERT_EQ(*x, *w);
}
