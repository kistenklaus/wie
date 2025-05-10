#include "container/container_concepts.hpp"
#include "memory/Mallocator.hpp"
#include "my_container.hpp"
#include <gtest/gtest.h>
#include <print>
#include <random>
#include <ranges>

// NOTE: This test is only applicable if the container is a std::range
// (see https://en.cppreference.com/w/cpp/ranges/range)
// You can gain extra points by also fullfilling the concepts
// - std::ranges::forward_range
// - std::ranges::bidirectional_range
// - std::ranges::random_access_range
// - std::ranges::contiguous_range
TEST(container_competition, iterable) {

  using Instance = MyContainer<float, strobe::Mallocator>;
  if (std::ranges::range<Instance>) {
    if (std::ranges::forward_range<Instance>) {
      if (std::ranges::bidirectional_range<Instance>) {
        if (std::ranges::random_access_range<Instance>) {
          if (std::ranges::contiguous_range<Instance>) {
            // Container is a contigous_range
            std::println("\033[1;34m[SUCCESS   ]\033[0m \033[1;36mYour "
                         "container is a contiguous range (+10 points)\033[0m");
          } else {
            // Container is a random access range
            std::println("\033[1;34m[SUCCESS   ]\033[0m \033[1;36mYour "
                         "container is a random access range (+9 points)\033[0m");
          }
        } else {
          // Container is a bidirectional range
          std::println("\033[1;34m[SUCCESS   ]\033[0m \033[1;36mYour container "
                       "is a bidirectional range (+8 points)\033[0m");
        }
      } else {
        // Container is forward range
        std::println("\033[1;34m[SUCCESS   ]\033[0m \033[1;36mYour container "
                     "is a forward range (+5 points)\033[0m");
      }
    } else {
      // Container is just a range
      std::println("\033[1;34m[SUCCESS   ]\033[0m \033[1;36mYour container is "
                   "a forward range (+3 points)\033[0m");
    }
  } else {
    std::println("\033[1;90m[SKIPPED   ]\033[0m \033[1;90mYour container is not a range (0 points)\033[0m");
  }
}
