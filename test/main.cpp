#include <gtest/gtest.h>

int main(int argc, char **argv) {
  ::testing::FLAGS_gtest_print_time = false; // Disable timing output
  ::testing::FLAGS_gtest_brief = true;
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
