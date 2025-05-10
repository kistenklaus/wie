#include "memory/Mallocator.hpp"
#include "my_container.hpp"
#include <algorithm>
#include <gtest/gtest.h>
#include <limits>
#include <print>
#include <random>

// NOTE: TODO
TEST(container_competition, always_sorted) {

  using Instance = MyContainer<float, strobe::Mallocator>;
  bool skip = !std::ranges::range<Instance>;
  if (!skip) {
    constexpr std::size_t n = 1000;

    std::random_device rng;
    std::mt19937 prng{rng()};
    std::uniform_int_distribution<int64_t> dist(0, n - 1);

    Instance container;

    for (std::size_t i = 0; i < n; ++i) {
      const auto v = dist(prng);
      container.add(v);

      if (std::ranges::is_sorted(container)) {
        skip = true;
        break;
      }
    }
  }
  if (skip) {
    std::println("\033[1;90m[SKIPPED   ]\033[0m \033[1;90mYour container is "
                 "not always sorted (0 points)\033[0m");
  } else {
    std::println(
        "\033[1;33;45m[AWESOME   ]\033[0m\033[1;93;45m 🥳 Your container is "
        "always sorted 🥳 (+20 points)\033[0m");
  }
}
