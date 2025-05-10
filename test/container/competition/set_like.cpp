#include "container/container_concepts.hpp"
#include "memory/Mallocator.hpp"
#include "my_container.hpp"
#include <gtest/gtest.h>
#include <print>
#include <random>

// NOTE: This test is only applicable if the container is a StackLikeContainer.
// i.e. it defines
// - push(const T& value)
// - top() -> T&
// - pop()
// - empty() -> bool
//
// Additionally requires a default constructor!
TEST(container_competition, set) {

  using Instance = MyContainer<float, strobe::Mallocator>;
  if constexpr (strobe::SetLikeContainer<Instance>) {

    static constexpr std::size_t n = 100;

    std::random_device rng;
    std::mt19937 prng{rng()};
    std::uniform_real_distribution<float> dist;
    std::uniform_int_distribution<std::uint64_t> valueDist;
    std::uniform_int_distribution<std::size_t> indexDist(0, n - 1);

    std::vector<uint64_t> values(n);
    std::vector<bool> added(n, false);
    for (auto &v : values) {
      v = valueDist(prng);
    }
    Instance container;

    for (std::size_t i = 0; i < n; ++i) {
      const auto index = indexDist(prng);
      const auto v = values[index];
      if (added[index]) {
        ASSERT_TRUE(container.contains(v));
        container.remove(v);
        ASSERT_FALSE(container.contains(v));
      } else {
        ASSERT_FALSE(container.contains(v));
        container.add(v);
        ASSERT_TRUE(container.contains(v));
      }
      added[index].flip();
    }

    std::println("\033[1;34m[SUCCESS   ]\033[0m \033[1;36mYour container "
                 "supports set interface (+10 points)\033[0m");

  } else {
    std::println("\033[1;90m[SKIPPED   ]\033[0m \033[1;90mContainer is not set "
                 "like (0 points)\033[0m");
  }
  ASSERT_EQ(true, true);
}
