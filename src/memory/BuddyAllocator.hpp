#pragma once

#include "memory/AllocatorTraits.hpp"
#include "memory/PageAllocator.hpp"
#include <bit>
#include <bitset>
#include <cassert>
#include <print>

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
    o.m_bitset.reset();
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
    o.m_bitset.reset();
    return *this;
  }

  void *allocate(std::size_t size, std::size_t align) {
    if (size == 0 || size > Capacity)
      return nullptr;
    std::size_t blockCount = (size + BlockSize - 1) >> LogBlockSize;
    std::size_t targetOrder = std::bit_width(blockCount - 1);
    std::size_t current = 0;
    std::size_t currentOrder = LogBlockCount;
    return allocateInternal(targetOrder, current, currentOrder);
  }

  void deallocate(void *ptr, std::size_t size, std::size_t) {
    if (ptr == nullptr) {
      return;
    }
    assert(owns(ptr));
    std::size_t offset = reinterpret_cast<std::byte *>(ptr) - m_buffer;
    std::size_t blockCount = (size + BlockSize - 1) >> LogBlockSize;
    std::size_t order = std::bit_width(blockCount - 1); //
    std::size_t invOrder = LogBlockCount - order;
    std::size_t orderNodeOffset = (1 << invOrder) - 1;
    std::size_t blockIdx = offset >> LogBlockSize;
    assert((offset % (BlockSize << order)) == 0);
    std::size_t nodeOffset = (blockIdx >> order) + orderNodeOffset;
    assert(nodeOffset < BlockCount * 2 - 1);
    /* assert(m_bitset.test(nodeOffset)); */
    m_bitset.reset(nodeOffset);
    order += 1;
    std::size_t current = (nodeOffset - 1) >> 1;
    while (order < LogBlockCount) {
      std::size_t left = current * 2 + 1;
      std::size_t right = left + 1;
      assert(left < BlockCount * 2 - 1);
      assert(right < BlockCount * 2 - 1);
      if (!m_bitset[left] || !m_bitset[right]) {
        m_bitset.reset(current);
      }
      current = (current - 1) >> 1;
      order += 1;
    }
  }

  bool owns(const void *p) const {
    const std::byte *raw = reinterpret_cast<const std::byte *>(p);
    return raw >= m_buffer && raw < m_buffer + Capacity;
  }

private:
  void *allocateInternal(std::size_t targetOrder, std::size_t current,
                         std::size_t currentOrder) {
    if (m_bitset[current]) {
      return nullptr;
    } else {
      if (targetOrder == currentOrder || currentOrder == 0) {
        // Actually allocate a block!
        std::size_t invCurrentOrder = LogBlockCount - currentOrder;
        std::size_t nodeCount = 1 << invCurrentOrder;
        std::size_t nodeOffset = nodeCount - 1;
        std::size_t blockIdx = current - nodeOffset;

        std::size_t blockSize = BlockSize << currentOrder;

        m_bitset.set(current);

        return m_buffer + blockSize * blockIdx;
      } else {

        // try to allocate from children
        std::size_t left = current * 2 + 1;
        std::size_t right = left + 1;
        { // try to allocate from left child
          void *p = allocateInternal(targetOrder, left, currentOrder - 1);
          if (p != nullptr) {
            if (m_bitset[left] && m_bitset[right]) {
              // merge buddies
              m_bitset.set(current);
            }
            return p;
          }
        }
        { // try to allocate from right child
          void *p = allocateInternal(targetOrder, right, currentOrder - 1);
          if (p != nullptr) {
            if (m_bitset[left] && m_bitset[right]) {
              // merge buddies
              m_bitset.set(current);
            }
            return p;
          }
        }
        return nullptr;
      }
    }
  }

  [[no_unique_address]] UpstreamAllocator m_upstream;

  std::bitset<BlockCount * 2ull - 1ull> m_bitset;
  std::byte *m_buffer;
};

} // namespace strobe
