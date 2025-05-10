#include "memory/Mallocator.hpp"
#include "my_container.hpp"
#include <gtest/gtest.h>
#include <print>
#include <random>
#include <type_traits>

// NOTE: This test is only applicable if the container is constructible from
// a range and it is move constructible
TEST(container_competition, move_constructible) {
  using Instance = MyContainer<float, strobe::Mallocator>;

  if constexpr (std::is_move_constructible_v<Instance>) {
    constexpr std::size_t n = 1000;
    std::vector<float> reference(n);

    std::mt19937 prng{std::random_device{}()};
    std::uniform_real_distribution<float> dist;
    std::generate(reference.begin(), reference.end(),
                  [&] { return dist(prng); });

    Instance container{reference};
    ASSERT_TRUE(std::ranges::equal(container, reference));

    Instance moved{std::move(container)};
    ASSERT_TRUE(std::ranges::equal(moved, reference));
    ASSERT_TRUE(container.empty() || std::ranges::equal(container, reference));

    std::println("\033[1;34m[SUCCESS   ]\033[0m \033[1;36mYour container "
                 "is move-constructible (+5 points)\033[0m");
  } else {
    std::println("\033[1;35m[SKIPPED   ]\033[0m \033[1;33mContainer is "
                 "not move-constructible (0 points)\033[0m");
  }
}

// NOTE: This test is only applicable if the container is constructible from
// a range and it is move assignable
TEST(container_competition, move_assignable) {
  using Instance = MyContainer<float, strobe::Mallocator>;

  if constexpr (std::is_move_assignable_v<Instance>) {
    constexpr std::size_t n = 1000;
    std::vector<float> reference(n);

    std::mt19937 prng{std::random_device{}()};
    std::uniform_real_distribution<float> dist;
    std::generate(reference.begin(), reference.end(),
                  [&] { return dist(prng); });

    Instance container{reference};
    ASSERT_TRUE(std::ranges::equal(container, reference));

    Instance moved;
    moved = std::move(container);

    ASSERT_TRUE(std::ranges::equal(moved, reference));
    ASSERT_TRUE(container.empty() || std::ranges::equal(container, reference));

    std::println("\033[1;34m[SUCCESS   ]\033[0m \033[1;36mYour container "
                 "is move-assignable (+5 points)\033[0m");
  } else {
    std::println("\033[1;90m[SKIPPED   ]\033[0m \033[1;90mContainer is "
                 "not move-assignable (0 points)\033[0m");
  }
}
