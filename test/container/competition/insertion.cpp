#include "./my_container.hpp"
#include "container/container_concepts.hpp"
#include "memory/Mallocator.hpp"
#include <algorithm>
#include <gtest/gtest.h>
#include <iterator>
#include <print>
#include <random>

// NOTE: This test is only applicable if the container fullfills
// ContainerSupportsInsertion. i.e. it defined a function
// - insert(const_iterator pos, const T& value);
TEST(container_competition, insert_element) {
  using Instance = MyContainer<float, strobe::Mallocator>;
  if constexpr (strobe::ContainerSupportsInsertion<Instance>) {

    static constexpr std::size_t n = 1000;
    std::random_device rng;
    std::mt19937 prng{rng()};
    std::uniform_real_distribution<float> dist;

    std::vector<float> reference;
    Instance container;

    for (std::size_t i = 0; i < n; ++i) {
      std::uniform_int_distribution<std::size_t> indexDist(0, reference.size());

      const std::size_t index = indexDist(prng);
      const auto v = dist(prng);

      if (index == reference.size()) {
        container.insert(container.end(), v);
        reference.insert(reference.end(), v);
      } else {
        auto pos = std::ranges::next(container.begin(), index);
        container.insert(pos, v);
        reference.insert(reference.begin() + index, v);
      }
      ASSERT_TRUE(std::ranges::equal(reference, container));
    }

    std::println("\033[1;34m[SUCCESS   ]\033[0m \033[1;36mYour container "
                 "supports element insertion (+10 points)\033[0m");
  } else {

    std::println("\033[1;35m[SKIPPED   ]\033[0m \033[1;33mContainer does not "
                 "support element insertion (0 points)\033[0m");
  }
}

// NOTE: Similar to but takes a range as input.
TEST(container_competition, insert_range) {
  using Instance = MyContainer<float, strobe::Mallocator>;
  using generic_range = std::list<float>;
  if constexpr (strobe::ContainerSupportsRangeInsertion<Instance, generic_range>) {

    static constexpr std::size_t n = 1000;
    std::random_device rng;
    std::mt19937 prng{rng()};
    std::uniform_real_distribution<float> dist;

    std::uniform_int_distribution<std::size_t> sizeDist(0,10);

    std::vector<float> reference;
    Instance container;

    for (std::size_t i = 0; i < n; ++i) {
      std::uniform_int_distribution<std::size_t> indexDist(0, reference.size());
      const std::size_t index = indexDist(prng);
      const std::size_t size = sizeDist(prng);

      generic_range temp(size);
      for (auto e : temp) {
        e = dist(prng);
      }

      if (index == reference.size()) {
        container.insert(container.end(), temp);
        reference.insert(reference.end(), std::ranges::begin(temp),
                         std::ranges::end(temp));
      } else {
        auto pos = std::ranges::next(container.begin(), index);
        container.insert(pos, temp);
        reference.insert(reference.begin() + index, std::ranges::begin(temp),
                         std::ranges::end(temp));
      }
      ASSERT_TRUE(std::ranges::equal(reference, container));
    }

    std::println("\033[1;34m[SUCCESS   ]\033[0m \033[1;36mYour container "
                 "supports range insertion (+10 points)\033[0m");
  } else {

    std::println("\033[1;90m[SKIPPED   ]\033[0m \033[1;90mContainer does not "
                 "support range insertion (0 points)\033[0m");
  }
}
