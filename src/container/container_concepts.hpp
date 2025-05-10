#pragma once

#include "memory/AllocatorTraits.hpp"
#include <concepts>
#include <list>
#include <ranges>
#include <utility>
namespace strobe {

template <typename T>
concept Container = requires {
  typename T::value_type;
  typename T::size_type;
  typename T::allocator_type;
} && strobe::Allocator<typename T::allocator_type>;

template <typename T>
concept RandomAccessContainer =
    Container<T> && requires(T &container, typename T::size_type i) {
      { container[i] } -> std::convertible_to<typename T::value_type &>;
    };

template <typename T>
concept StackLikeContainer =
    Container<T> && requires(T &container, const typename T::value_type &v) {
      { container.push(v) };
      { container.top() } -> std::convertible_to<typename T::value_type &>;
      { container.pop() };
      { container.empty() } -> std::convertible_to<bool>;
    };

template <typename T>
concept SetLikeContainer =
    Container<T> && requires(T &container, const typename T::value_type &v) {
      { container.contains(v) } -> std::convertible_to<typename T::value_type>;
      { container.empty() } -> std::convertible_to<bool>;
      { container.add(v) };
      { container.remove(v) } -> std::convertible_to<bool>;
    };

template <typename T>
concept QueueLikeContainer =
    Container<T> && requires(T &container, const typename T::value_type &v) {
      { container.enqueue(v) };
      {
        const_cast<const T &>(container).peek()
      } -> std::convertible_to<const typename T::value_type &>;
      { container.dequeue() } -> std::convertible_to<typename T::value_type>;
      { container.empty() } -> std::convertible_to<bool>;
    };

template <typename T>
concept ContainerSupportsInsertion =
    Container<T> && std::ranges::range<T> &&
    requires(T &container, const typename T::value_type &v) {
      { container.insert(std::declval<typename T::const_iterator>(), v) };
    };

template <typename T, typename Rg>
concept ContainerSupportsRangeInsertion =
    std::ranges::range<T> && std::ranges::range<Rg> &&
    std::same_as<std::ranges::range_value_t<Rg>, typename T::value_type> &&
    requires(T &container, typename T::const_iterator pos, Rg rg) {
      { container.insert(pos, rg) } -> std::same_as<typename T::iterator>;
    };

} // namespace strobe
