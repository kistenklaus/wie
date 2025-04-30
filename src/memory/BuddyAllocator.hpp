#pragma once

#include "memory/AllocatorTraits.hpp"
#include "memory/PageAllocator.hpp"
#include <array>
#include <bit>
#include <bitset>
#include <cassert>
#include <print>
#include <stdexcept>
#include <vector>

namespace strobe {

template <std::size_t Capacity, std::size_t BlockSize,
          typename UpstreamAllocator = PageAllocator>
class BuddyResource {
private:
  using UpstreamTraits = AllocatorTraits<UpstreamAllocator>;
  static_assert(std::has_single_bit(Capacity));
  static_assert(std::has_single_bit(BlockSize));
  static_assert(BlockSize < Capacity);

  static constexpr std::size_t LogCapacity = std::bit_width(Capacity - 1);

  static constexpr std::size_t LogBlockSize = std::bit_width(BlockSize - 1);
  static constexpr std::size_t LogBlockCount = LogCapacity - LogBlockSize;
  static constexpr std::size_t BlockCount = 1ull << LogBlockCount;

  struct FreelistNode {
    struct FreelistNode* next;
    struct FreelistNode* prev;
  };

public:
  BuddyResource(UpstreamAllocator upstream = {})
      : m_upstream(std::move(upstream)) {
    m_buffer = reinterpret_cast<std::byte *>(UpstreamTraits::allocate(
        m_upstream, Capacity, alignof(std::max_align_t)));
  }
  ~BuddyResource() {
    if (m_buffer != nullptr) {
      UpstreamTraits::deallocate(m_upstream, m_buffer, Capacity,
                                 alignof(std::max_align_t));
      m_buffer = nullptr;
    }
  }
  BuddyResource(const BuddyResource &) = delete;
  BuddyResource &operator=(const BuddyResource &) = delete;
  BuddyResource(BuddyResource &&o)
      : m_buffer(o.m_buffer), m_bitset(o.m_bitset) {
    o.m_buffer = nullptr;
    o.m_bitset.clear();
  }

  BuddyResource &operator=(BuddyResource &&o) {
    if (this == &o) {
      return *this;
    }
    if (m_buffer != nullptr) {
      UpstreamTraits::deallocate(m_upstream, m_buffer, Capacity,
                                 alignof(std::max_align_t));
    }
    m_buffer = o.m_buffer;
    m_bitset = o.m_bitset;
    o.m_buffer = nullptr;
    o.m_bitset.clear();
    return *this;
  }

  void *allocate(std::size_t size, std::size_t) {
    size = std::bit_ceil(size);
    if (size == 0 || size > Capacity)
      return nullptr;
    std::size_t blockCount = (size + BlockSize - 1) >> LogBlockSize;
    std::size_t targetOrder = LogBlockCount - std::bit_width(blockCount - 1);
    return allocateInternal(targetOrder);
  }

  void deallocate(void *ptr, std::size_t size, std::size_t) {
    if (ptr == nullptr) {
      return;
    }
    size = std::bit_ceil(size);
    if (size == 0 || size > Capacity) {
      throw std::runtime_error("Invalid arguments to deallocate");
    }
    std::size_t log2Size = std::bit_width(size - 1);
    std::size_t logBlockCount = log2Size - LogBlockSize;
    assert(owns(ptr));
    std::size_t offset = reinterpret_cast<std::byte *>(ptr) - m_buffer;
    std::size_t blockOffset = offset / size;
    std::size_t order = LogBlockCount - logBlockCount;
    std::size_t block = blockOffset + ((1 << order) - 1);
    assert(m_bitset[block]);
    while (block != 0) {
      m_bitset[block] = false;
      std::size_t buddy;
      if (block & 0x1) { // is left child
        buddy = block + 1;
      } else {
        buddy = block - 1;
      }
      if (m_bitset[buddy] == false) {
        block = (block - 1) / 2;
        continue;
      }
      return;
    }
    assert(block == 0);
    m_bitset[0] = false;
  }

  bool owns(const void *p) const {
    const std::byte *raw = reinterpret_cast<const std::byte *>(p);
    return raw >= m_buffer && raw < m_buffer + Capacity;
  }

private:
  std::size_t searchForBlock(std::size_t targetOrder) {

    std::array<std::size_t, 2 * LogBlockCount> stack;
    stack[0] = 0;
    std::size_t stackSize = 1;
    while (stackSize > 0) {
      // pop current from stack
      std::size_t current = stack[--stackSize];
      std::size_t order = std::bit_width(current + 1) - 1;
      // std::println("current = {}, order = {} target={}", current, order,
      // targetOrder);

      if (order == targetOrder) {
        if (m_bitset[current]) {
          continue;
        } else {
          stack[stackSize++] = current;
          break;
        }
      }

      std::size_t left = 2 * current + 1;
      std::size_t right = left + 1;

      if (m_bitset[current] && !m_bitset[left] && !m_bitset[right]) {
        continue;
      }
      // std::println("left = {}, right = {}", left, right);

      stack[stackSize++] = right;
      stack[stackSize++] = left;
    }
    if (stackSize == 0) {
      // no block was found
      return -1;
    }
    return stack[stackSize - 1];
  }

  void mergeBuddiesUp(std::size_t block) {
    m_bitset[block] = true;
    block = (block - 1) / 2;
    while (block < m_bitset.size()) {
      if (m_bitset[block]) {
        break;
      } else {
        m_bitset[block] = true;
        block = (block - 1) / 2;
      }
    }
  }

  void *allocateInternal(std::size_t targetOrder) {
    std::size_t block = searchForBlock(targetOrder);
    if (block == -1) {
      return nullptr;
    }
    assert(m_bitset[block] == false);
    assert(std::bit_width(block + 1) - 1 == targetOrder);
    std::size_t order = targetOrder;
    std::size_t offset =
        (block - ((1 << order) - 1)) * (BlockSize << (LogBlockCount - order));
    void *ptr = m_buffer + offset;
    mergeBuddiesUp(block);
    return ptr;
  }

  [[no_unique_address]] UpstreamAllocator m_upstream;

  std::bitset<BlockCount * 2> m_bitset;
  std::byte *m_buffer;
  std::array<FreelistNode, BlockCount / 2> m_freelistStorage;
  std::array<FreelistNode*, LogBlockCount> m_freelists;
};

} // namespace strobe
