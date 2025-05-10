#include "container/container_concepts.hpp"
#include "memory/Mallocator.hpp"
#include "my_container.hpp"
#include <gtest/gtest.h>
#include <print>

// NOTE: This test is only applicable if the container is constructible from
// a range and it is copy constructible
TEST(container_competition, immutable) {
  using Instance = MyContainer<float, strobe::Mallocator>;
  if ((strobe::Container<Instance> && !strobe::StackLikeContainer<Instance> &&
       !strobe::SetLikeContainer<Instance> &&
       !strobe::ContainerSupportsInsertion<Instance> &&
       strobe::ContainerSupportsRangeInsertion<Instance, std::list<float>>)) {
    std::println(
        "\033[1;33;45m[AWESOME   ]\033[0m\033[1;93;45m 🥳 Your container is "
        "immutable 🥳 (+35 points)\033[0m");
  } else {
    std::println("\033[1;90m[SKIPPED   ]\033[0m \033[1;90mContainer is not "
                 "immutable (0 points)\033[0m");
  }
}
