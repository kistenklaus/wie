#include "memory/Mallocator.hpp"
#include "my_container.hpp"
#include <algorithm>
#include <gtest/gtest.h>
#include <print>
#include <random>
#include <type_traits>

// NOTE: This test is only applicable if the container is constructible from
// a range and it is copy constructible
TEST(container_competition, copy_constructible) {
  using Instance = MyContainer<float, strobe::Mallocator>;
  if (std::is_copy_constructible_v<Instance>) {

    static constexpr std::size_t n = 1000;
    std::vector<float> reference(n);

    std::random_device rng;
    std::mt19937 prng{rng()};
    std::uniform_real_distribution<float> dist;
    for (auto &v : reference) {
      v = dist(prng);
    }

    Instance container{reference};
    ASSERT_TRUE(std::ranges::equal(container, reference));

    Instance copy{container};

    ASSERT_TRUE(std::ranges::equal(container, copy));

    std::println("\033[1;34m[SUCCESS   ]\033[0m \033[1;36mYour container is "
                 "copy constructible (+5 points)\033[0m");
  } else {
    std::println("\033[1;90m[SKIPPED   ]\033[0m \033[1;90mContainer is not "
                 "copy constructible (0 points)\033[0m");
  }
}

// NOTE: This test is only applicable if the container is constructible from
// a range and it is copy assignable
TEST(container_competition, copy_assignable) {
  using Instance = MyContainer<float, strobe::Mallocator>;
  if (std::is_copy_assignable_v<Instance>) {

    static constexpr std::size_t n = 1000;
    std::vector<float> reference(n);

    std::random_device rng;
    std::mt19937 prng{rng()};
    std::uniform_real_distribution<float> dist;
    for (auto &v : reference) {
      v = dist(prng);
    }

    Instance container{reference};
    ASSERT_TRUE(std::ranges::equal(container, reference));

    Instance copy;
    copy = container;

    ASSERT_TRUE(std::ranges::equal(container, copy));

    std::println("\033[1;34m[SUCCESS   ]\033[0m \033[1;36mYour container is "
                 "copy assignable (+5 points)\033[0m");
  } else {
    std::println("\033[1;35m[SKIPPED   ]\033[0m \033[1;33mContainer is not "
                 "copy assignable (0 points)\033[0m");
  }
}
