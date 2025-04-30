#pragma once

#include <cassert>
#include <cstddef>
namespace strobe {

inline std::size_t align_up(std::size_t offset, std::size_t alignment) noexcept {
  assert(alignment && (alignment & (alignment - 1)) == 0 && "alignment must be power of two");
  return (offset + alignment - 1) & ~(alignment - 1);
}

}
