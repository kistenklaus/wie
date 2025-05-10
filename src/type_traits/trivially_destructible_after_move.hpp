#pragma once

#include <type_traits>
namespace strobe {

/// Is std::true_type iff. the type T is trivially destructible after beeing
/// moved. i.e. after a move a instance of T is in state, which
/// is specified to be trivially destructible.
/// This is a addition to the C++ standard where the state after a move is
/// explicitly defined as undefined behavior. 
/// NOTE: Keep in mind that compilers can always elide moves!
template <typename T>
struct is_trivially_destructible_after_move
    : std::is_trivially_destructible<T> {};

template <typename T>
inline constexpr bool is_trivially_destructible_after_move_v =
    is_trivially_destructible_after_move<T>::value;

} // namespace strobe
