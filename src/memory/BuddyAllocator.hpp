#pragma once

#include "memory/AllocatorTraits.hpp"
#include "memory/PageAllocator.hpp"
#include <array>
#include <bit>
#include <bitset>
#include <cassert>
#include <cstddef>
#include <stdexcept>

namespace strobe {

template <std::size_t Capacity, std::size_t BlockSize,
          typename UpstreamAllocator = PageAllocator>
class BuddyAllocator {
  using UpstreamTraits = AllocatorTraits<UpstreamAllocator>;
  static_assert(std::has_single_bit(Capacity));
  static_assert(std::has_single_bit(BlockSize));
  static_assert(BlockSize < Capacity);

  static constexpr std::size_t LogCapacity = std::bit_width(Capacity - 1);

  static constexpr std::size_t LogBlockSize = std::bit_width(BlockSize - 1);
  static constexpr std::size_t LogBlockCount = LogCapacity - LogBlockSize;
  static constexpr std::size_t BlockCount = 1ull << LogBlockCount;

  struct FreelistNode {
    FreelistNode *next;
    FreelistNode *prev;
  };

public:
  explicit BuddyAllocator(UpstreamAllocator upstream = {})
      : m_upstream(std::move(upstream)) {
    m_buffer = reinterpret_cast<std::byte *>(UpstreamTraits::allocate(
        m_upstream, Capacity, alignof(std::max_align_t)));
    for (FreelistNode &node : m_freelistStorage) {
      node.next = nullptr;
      node.prev = nullptr;
    }
    for (FreelistNode *&ptr : m_freelists) {
      ptr = nullptr;
    }
    pushFreelist(0, &getFreelistNode(0, 0));
  }
  ~BuddyAllocator() {
    if (m_buffer != nullptr) {
      UpstreamTraits::deallocate(m_upstream, m_buffer, Capacity,
                                 alignof(std::max_align_t));
      m_buffer = nullptr;
    }
  }
  BuddyAllocator(const BuddyAllocator &) = delete;
  BuddyAllocator &operator=(const BuddyAllocator &) = delete;
  BuddyAllocator(BuddyAllocator &&o) = delete;

  BuddyAllocator &operator=(BuddyAllocator &&o) = delete;


  bool owns(const void *p) const {
    const auto *raw = static_cast<const std::byte *>(p);
    return raw >= m_buffer && raw < m_buffer + Capacity;
  }

  void *allocate(std::size_t size, std::size_t alignment) {
    assert(alignment <= size && std::has_single_bit(alignment));
    size = std::bit_ceil(size);
    if (size == 0 || size > Capacity) {
      return nullptr;
    }
    const std::size_t block = size >> LogBlockSize;
    const int log2Block = floorLog2(block);
    const int order = LogBlockCount - log2Block;
    void *ptr = allocateFromFreelist(order);
    return ptr;
  }

  void deallocate(void *ptr, std::size_t size, std::size_t) {
    size = std::bit_ceil(size);
    if (size == 0 || size > Capacity) {
      throw std::runtime_error("Invalid size for deallocate");
    }
    const std::size_t block = size >> LogBlockSize;
    const int log2Block = floorLog2(block);
    const int order = LogBlockCount - log2Block;
    deallocateToFreelist(ptr, order);
  }

private:
  static constexpr int floorLog2(const std::size_t n) { return std::bit_width(n) - 1; }

  static std::size_t indexOffsetOfOrder(const int order) {
    return (1 << static_cast<std::size_t>(order)) - 1;
  }
  static std::size_t rankOfNodeIndex(const std::size_t nodeIndex, const int order) {
    const std::size_t offset = indexOffsetOfOrder(order);
    return nodeIndex - offset;
  }
  static std::size_t leftChild(const std::size_t index) { return 2 * index + 1; }
  static std::size_t leftChildN(const std::size_t index, const std::size_t n) {
    return ((index + 1) << n) - 1;
  }
  static std::size_t rightChild(const std::size_t index) { return 2 * index + 2; }
  static std::size_t parentOfIndex(const std::size_t index) {
    return (index - 1) / 2;
  }

  FreelistNode &getFreelistNode(const std::size_t index, const int order) {
    if (order == LogBlockCount) {
      const std::size_t orderOffset = (1 << order) - 1;
      assert(orderOffset <= index);
      const std::size_t block = index - orderOffset;
      return m_freelistStorage[block / 2];
    }
    const std::size_t rank = rankOfNodeIndex(index, order);
    std::size_t idx = rank << (LogBlockCount - order - 1);
    return m_freelistStorage[idx];
  }

  FreelistNode *popFreelist(int order) {
    FreelistNode *head = m_freelists[order];
    if (head == nullptr) {
      return nullptr;
    }
    FreelistNode *next = head->next;
    m_freelists[order] = next;
    if (next != nullptr) {
      next->prev = nullptr;
    }
    // TODO might not be needed
    head->next = nullptr;
    head->prev = nullptr;
    return head;
  }

  void eraseNodeFromFreelist(FreelistNode* node, int order) {
    if (node->prev == nullptr) {
      m_freelists[order] = nullptr;
    } else {
      FreelistNode* prev = node->prev;
      prev->next = node->next;
      // TODO Maybe we don't have to cleanup non used nodes.
      node->next = nullptr;
      node->prev = nullptr;
    }
  }

  static FreelistNode *rightChildOfFreelistNode(FreelistNode *node,
                                         const int order) {
    if (order == LogBlockCount - 1) {
      return node;
    }
    assert(order != LogBlockCount - 1);
    const std::size_t shift = LogBlockCount - order - 2;
    return node + (1 << shift);
  }

  std::size_t freelistPtrToIndex(FreelistNode *node, const int order) {
    if (order == LogBlockCount) {
      // NOTE: Requires bitset because freelist pointers only give us half
      // resolution!
      const std::size_t location = (node - m_freelistStorage.data());
      const std::size_t left = location * 2;
      const std::size_t offset = indexOffsetOfOrder(order);
      std::size_t leftIndex = offset + left;
      if (m_bitset[leftIndex]) {
        return leftIndex + 1;
      }
      if (m_bitset[leftIndex + 1]) {
        return leftIndex;
      }
    }
    const std::size_t location = (node - m_freelistStorage.data());
    const std::size_t shift = LogBlockCount - order - 1;
    const std::size_t idx = location >> shift;
    const std::size_t offset = indexOffsetOfOrder(order);
    return offset + idx;
  }

  std::size_t ptrToIndex(void *ptr, const int order) const {
    const auto *raw = static_cast<std::byte *>(ptr);
    const std::ptrdiff_t diff = raw - m_buffer;
    const std::size_t block = diff >> LogBlockSize;
    const std::size_t rank = block >> (LogBlockCount - order);
    const std::size_t offset = indexOffsetOfOrder(order);
    return offset + rank;
  }

  static constexpr std::size_t buddyOfIndex(const std::size_t index) {
    assert(index != 0);
    if (index & 0x1) { // is left child.
      return index + 1;
    }
    // is right child.
    return index - 1;
  }

  void pushFreelist(const int order, FreelistNode *node) {
    assert(node != nullptr);
    FreelistNode *head = m_freelists[order];
    m_freelists[order] = node;
    if (head != nullptr) {
      head->prev = node;
    }
    node->next = head;
    node->prev = nullptr;
  }

  void *allocateFromFreelist(const int order) {
    FreelistNode *node = nullptr;
    int o = order;
    while (o >= 0) {
      node = popFreelist(o);
      if (node == nullptr) {
        --o;
      } else {
        break;
      }
    }
    if (node == nullptr) {
      return nullptr;
    }

    // Break up blocks
    std::size_t index = freelistPtrToIndex(node, o);
    const std::size_t rank = rankOfNodeIndex(index, o);
    const std::size_t ptrOffset = (rank << (LogBlockCount - o)) << LogBlockSize;
    m_bitset.set(index);
    while (o != order) {
      index = index * 2 + 1;
      m_bitset.set(index);
      FreelistNode *right = rightChildOfFreelistNode(node, o);
      //assert(right >= &m_freelistStorage.front());
      //assert(right < &m_freelistStorage.back());
      ++o;
      pushFreelist(o, right);
    }
    return m_buffer + ptrOffset;
  }

  void deallocateToFreelist(void *ptr, const int order) {
    std::size_t index = ptrToIndex(ptr, order);
    //assert(m_bitset[index] == true);

    std::size_t o = order;
    while (index != 0) {
      m_bitset.reset(index);
      std::size_t buddy = buddyOfIndex(index);
      if (m_bitset[buddy]) {
        break;
      }
      // NOTE: Coalesce buddies
      FreelistNode& node = getFreelistNode(buddy, o);
      eraseNodeFromFreelist(&node, o);
      index = parentOfIndex(index);
      --o;
    }
    if (index == 0) {
      m_bitset.reset(0);
    }
    FreelistNode& node = getFreelistNode(index, o);
    pushFreelist(o, &node);
  }

  UpstreamAllocator m_upstream;

  std::bitset<BlockCount * 2> m_bitset;
  std::byte *m_buffer;
  std::array<FreelistNode, BlockCount / 2> m_freelistStorage;
  std::array<FreelistNode *, LogBlockCount + 1> m_freelists;
};

} // namespace strobe
