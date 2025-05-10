#include "./my_container.hpp"
#include "container/container_concepts.hpp"
#include "memory/Mallocator.hpp"
#include <gtest/gtest.h>
#include <print>
#include <queue>
#include <random>

// NOTE: TODO
TEST(container_competition, fifo_queue_like) {
  using Instance = MyContainer<float, strobe::Mallocator>;

  if constexpr (strobe::QueueLikeContainer<Instance>) {

    constexpr std::size_t n = 20000;

    std::random_device rng;
    std::mt19937 prng{rng()};
    std::uniform_real_distribution<float> dist;

    std::vector<float> reference;
    Instance container;

    bool ok = true;

    for (std::size_t i = 0; i < n; ++i) {
      const auto u = dist(prng);
      if (u > 0.75 || container.empty()) {
        container.enqueue(u);
        reference.push_back(u);
      } else {
        const auto v = container.dequeue();
        const auto &e = reference.front();
        if (e != v) {
          ok = false;
          break;
        }
        reference.erase(reference.begin());
      }
    }
    if (ok) {
      // Container is forward range
      std::println("\033[1;34m[SUCCESS   ]\033[0m \033[1;36mYour container "
                   "behaves like a fifo queue (+10 points)\033[0m");
    } else {
      std::println(
          "\033[1;90m[SKIPPED   ]\033[0m \033[1;90mContainer is does not "
          "behave like a fifo queue (0 points)\033[0m");
    }

  } else {
    std::println(
        "\033[1;90m[SKIPPED   ]\033[0m \033[1;90mContainer is does not "
        "support a queue like interface (0 points)\033[0m");
  }
}

// NOTE: TODO
TEST(container_competition, priority_queue_like) {
  using Instance = MyContainer<float, strobe::Mallocator>;

  if constexpr (strobe::QueueLikeContainer<Instance>) {

    constexpr std::size_t n = 20000;

    std::random_device rng;
    std::mt19937 prng{rng()};
    std::uniform_real_distribution<float> dist;

    std::priority_queue<float> reference;
    Instance container;

    bool ok = true;

    for (std::size_t i = 0; i < n; ++i) {
      const auto u = dist(prng);
      if (u > 0.75 || container.empty()) {
        container.enqueue(u);
        reference.push(u);
      } else {
        const auto v = container.dequeue();
        const auto &e = reference.top();
        if (e != v) {
          ok = false;
          break;
        }
        reference.pop();
      }
    }
    if (ok) {
      std::println("\033[1;33;45m[AWESOME   ]\033[0m\033[1;93;45m 🥳 Your "
                   "container behaves "
                   "like a priority queue 🥳 (+30 points)\033[0m");
    } else {
      std::println(
          "\033[1;90m[SKIPPED   ]\033[0m \033[1;90mContainer is does not "
          "behave like a priority queue (0 points)\033[0m");
    }

  } else {
    std::println(
        "\033[1;90m[SKIPPED   ]\033[0m \033[1;90mContainer is does not "
        "support a queue like interface (0 points)\033[0m");
  }
}
