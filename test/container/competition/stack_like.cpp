#include "container/container_concepts.hpp"
#include "memory/Mallocator.hpp"
#include <print>
#include "my_container.hpp"
#include <gtest/gtest.h>
#include <random>

// NOTE: This test is only applicable if the container is a StackLikeContainer.
// i.e. it defines
// - push(const T& value)
// - top() -> T&
// - pop()
// - empty() -> bool
//
// Additionally requires a default constructor!
TEST(container_competition, stack) {

  using Instance = MyContainer<float, strobe::Mallocator>;
  if constexpr (strobe::StackLikeContainer<Instance>) {
    std::random_device rng;
    std::mt19937 prng{rng()};
    std::uniform_real_distribution<float> dist;

    std::vector<float> reference;
    Instance container;

    static constexpr std::size_t n = 10000;
    for (std::size_t i = 0; i < n; ++i) {
      EXPECT_EQ(container.empty(), reference.empty());
      const auto u = dist(prng);
      if (u > 0.75 && !container.empty()) {
        const auto value = container.top();
        const auto expected = reference.back();
        EXPECT_EQ(value, expected);
        container.pop();
        reference.pop_back();
      } else {
        container.push_back(u);
        reference.push_back(u);
      }
    }

    std::println("\033[1;34m[SUCCESS   ]\033[0m \033[1;36mYour container supports stack interface (+10 points)\033[0m");

  } else {
    std::println("\033[1;90m[SKIPPED   ]\033[0m \033[1;90mContainer is not stack like (0 points)\033[0m");
  }
  ASSERT_EQ(true, true);
}
