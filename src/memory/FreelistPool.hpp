#pragma once

#include "memory/AllocatorTraits.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
namespace strobe {

template <std::size_t BlockSize, std::size_t Alignment,
          Allocator UpstreamAllocator>
class FreelistResource {

  struct FreelistNode {
    FreelistNode *next;
  };

  union alignas(Alignment) Chunk {
    FreelistNode free;
    std::byte allocated[BlockSize];
  };

  static constexpr std::size_t ChunkSize = sizeof(Chunk);

public:
  explicit FreelistResource(UpstreamAllocator upstream, std::size_t blockCount)
      : m_upstream(std::move(upstream)), m_chunkCount(blockCount),
        m_chunks(UpstreamTraits::allocate(m_upstream, m_chunkCount * ChunkSize,
                                          Alignment)) {}

  ~FreelistResource() {
    if (m_chunks != nullptr) {
      UpstreamAllocator::deallocate(m_upstream, m_chunks,
                                    m_chunkCount * ChunkSize, Alignment);
      m_chunks = nullptr;
    }

    // Setup freelist
    FreelistNode *prev = nullptr;
    for (std::int64_t i = m_chunkCount - 1; i >= 0; --i) {
      m_chunks[i].free.next = prev;
      prev = &m_chunks[i].free;
    }
    m_freelist = prev;
  }

  FreelistResource(const FreelistResource &) = delete;
  FreelistResource &operator=(const FreelistResource &) = delete;

  FreelistResource(FreelistResource &&o)
      : m_upstream(std::move(o.m_upstream)), m_chunkCount(o.m_chunkCount),
        m_chunks(std::exchange(o.m_chunks, nullptr)) {}

  FreelistResource &operator=(FreelistResource &&o) {
    if (this == &o) {
      return *this;
    }
    if (m_chunks != nullptr) {
      UpstreamAllocator::deallocate(m_upstream, m_chunks,
                                    m_chunkCount * ChunkSize, Alignment);
    }
    m_upstream = std::move(o.m_upstream);
    m_chunkCount = o.m_chunkCount;
    m_chunks = std::exchange(o.m_chunks, nullptr);
    return *this;
  }

  void *allocate(std::size_t size, std::size_t align) {
    assert(size <= BlockSize);
    assert(align % Alignment == 0);

    // TODO
    return nullptr;
  }

  void deallocate(void *ptr, std::size_t size, std::size_t align) {
    assert(size <= BlockSize);
    assert(align % Alignment == 0);

    // TODO
  }

  bool owns(void *ptr) {
    // TODO!
    return false;
  }

private:
  using UpstreamTraits = AllocatorTraits<UpstreamAllocator>;
  [[no_unique_address]] UpstreamAllocator m_upstream;
  std::size_t m_chunkCount;
  Chunk *m_chunks;
  Chunk *m_freelist;
};

} // namespace strobe
