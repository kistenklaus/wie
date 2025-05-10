#include "./my_container.hpp"
#include "container/container_concepts.hpp"
#include "memory/Mallocator.hpp"
#include <gtest/gtest.h>
#include <print>
#include <random>

// NOTE: This test is only applicable if the container is a random access
// container i.e. it defined the operator[](std::size_t i).
// Whenever we test a value with operator[] we expect the value to not change
// regardless of if we write to other entries!
//
// We additionally require that the container defined a constructor which takes
// a inital size, the values within the container can be undefined after
// construction!
//
TEST(container_competition, random_access) {
  // container with 10 elements!
  using Instance = MyContainer<float, strobe::Mallocator>;
  if constexpr (strobe::RandomAccessContainer<Instance>) {
    constexpr std::size_t n = 1000;

    std::random_device rng;
    std::mt19937 prng{rng()};
    std::uniform_real_distribution<float> dist;
    std::uniform_int_distribution<std::size_t> indexDist(0, n - 1);

    std::vector<std::optional<float>> reference(n, std::nullopt);
    Instance container(n);

    std::size_t iterations = n * 10;
    for (std::size_t i = 0; i < iterations; ++i) {
      std::size_t index = indexDist(prng);
      if (!reference[index].has_value()) {
        float v = dist(prng);
        container[index] = v;
        reference[index] = v;
      } else {
        ASSERT_EQ(reference[index].value(), container[index]) << " \"index\" is equal to " << index;
        reference[index] = std::nullopt;
      }
    }
    std::println("\033[1;34m[SUCCESS   ]\033[0m \033[1;36mYour container supports random access (+10 points)\033[0m");
  } else {

    std::println("\033[1;90m[SKIPPED   ]\033[0m \033[1;90mContainer is not stack like (0 points)\033[0m");
  }
}
