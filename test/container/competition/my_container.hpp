#pragma once
#include "container/vector.hpp"
#include "memory/AllocatorTraits.hpp"

template<typename T, strobe::Allocator A>
using MyContainer = strobe::Vector<T,A>;





