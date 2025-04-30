#pragma once

#include <cassert>
#include <concepts>
#include <type_traits>

#include "memory/UseCounter.hpp"
#include "memory/AllocatorTraits.hpp"
#include "memory/FlexibleLayout.hpp"
#include "memory/align.hpp"

namespace strobe {

namespace detail {

template <typename T, Allocator A>
struct SharedControlBlock {
 private:
  using RefCounter = UseCounter<unsigned long>;

 public:
  T body;  // 8 byte
  struct {
    RefCounter useCounter;              // 8 byte
    [[no_unique_address]] A allocator;  // 1 byte ???
  } header;

  template <typename... Args>
  static SharedControlBlock* make(const A& alloc, Args&&... args) {
    using ATraits = AllocatorTraits<A>;
    A allocCopy = alloc;

    SharedControlBlock* block;
    if constexpr (std::derived_from<T, FlexibleLayout>) {
      TailAllocator tailAlloc{PolyAllocator(&allocCopy),
                              sizeof(SharedControlBlock<T, A>),
                              alignof(SharedControlBlock<T, A>)};
      if constexpr (SizeIndepdententAllocator<A>) {
        new (&block->body) T(tailAlloc, std::forward<Args>(args)...);
      } else {
        auto allocationSize = tailAlloc.reserve<std::size_t>();
        new (&block->body) T(tailAlloc, std::forward<Args>(args)...);
        allocationSize.set(tailAlloc.get(), tailAlloc.totalSize());
      }

      block = reinterpret_cast<SharedControlBlock<T, A>*>(tailAlloc.header());

    } else {
      block = ATraits::template allocate<SharedControlBlock>(allocCopy);
      new (&block->body) T(std::forward<Args>(args)...);
    }

    new (&block->header.useCounter) RefCounter();
    block->header.allocator = allocCopy;

    return block;
  }

  static void free(SharedControlBlock* block) {
    A alloc = block->header.allocator;
    block->~SharedControlBlock();  // destruct
    using ATraits = AllocatorTraits<A>;
    if constexpr (std::derived_from<T, FlexibleLayout>) {
      if constexpr (SizeIndepdententAllocator<A>) {
        ATraits::size_independent_deallocate(alloc, block);
      } else {
        std::size_t allocationSizeOffset = strobe::align_up(
            sizeof(SharedControlBlock<T, A>), alignof(std::size_t));
        std::size_t allocationSize = *reinterpret_cast<std::size_t*>(
            reinterpret_cast<std::byte*>(block) + allocationSizeOffset);

        ATraits::deallocate(alloc, block, allocationSize,
                            alignof(SharedControlBlock<T, A>));
      }
    } else {
      ATraits::template deallocate<SharedControlBlock>(alloc, block);
    }
  }
};

}  // namespace detail

template <typename T, Allocator A>
  requires(std::is_final_v<T>)
class SharedBlock {
 public:
  ~SharedBlock() { release(); }
  SharedBlock(const SharedBlock& o) : m_controlBlock(o.m_controlBlock) {
    if (m_controlBlock != nullptr) {
      m_controlBlock->header.useCounter.inc();
    }
  }
  SharedBlock& operator=(const SharedBlock& o) {
    if (m_controlBlock == o.m_controlBlock) {
      return *this;
    }
    release();
    if (o.m_controlBlock != nullptr) {
      if (m_controlBlock->header.useCounter.inc()) {
        m_controlBlock = o.m_controlBlock;
      }
    }
    return *this;
  }

  SharedBlock(SharedBlock&& o)
      : m_controlBlock(std::exchange(o.m_controlBlock, nullptr)) {}
  SharedBlock& operator=(SharedBlock&& o) {
    if (m_controlBlock == o.m_controlBlock) {
      return *this;
    }
    release();
    m_controlBlock = std::exchange(o.m_controlBlock, nullptr);
    return *this;
  }

  T* get() noexcept {
    return m_controlBlock == nullptr ? nullptr : &m_controlBlock->body;
  }

  const T* get() const noexcept {
    return m_controlBlock == nullptr ? nullptr : &m_controlBlock->body;
  }

  T& operator*() noexcept {
    assert(m_controlBlock != nullptr);
    return m_controlBlock->body;
  }
  const T& operator*() const noexcept {
    assert(m_controlBlock != nullptr);
    return m_controlBlock->body;
  }

  T* operator->() noexcept {
    assert(m_controlBlock != nullptr);
    return &m_controlBlock->body;
  }

  const T* operator->() const noexcept {
    assert(m_controlBlock != nullptr);
    return &m_controlBlock->body;
  }

  void pin() noexcept {
    assert(m_controlBlock != nullptr);
    bool ok = m_controlBlock->header.useCounter.inc();
    assert(ok);
  }

  void unpin() noexcept {
    assert(m_controlBlock != nullptr);
    if (m_controlBlock->header.useCounter.dec()) {
      detail::SharedControlBlock<T, A>::free(m_controlBlock);
      m_controlBlock = nullptr;
    }
  }

  auto useCount() const noexcept {
    assert(m_controlBlock != nullptr);
    return m_controlBlock->header.useCounter.useCount();
  }

  void reset() { release(); }

  void swap(SharedBlock<T, A>& o) {
    std::swap(m_controlBlock, o.m_controlBlock);
  }

  bool operator==(const SharedBlock& other) const noexcept {
    return m_controlBlock == other.m_controlBlock;
  }

  bool operator!=(const SharedBlock& other) const noexcept {
    return m_controlBlock != other.m_controlBlock;
  }

  auto operator<=>(const SharedBlock& other) const noexcept {
    return m_controlBlock <=> other.m_controlBlock;
  }

  bool operator==(std::nullptr_t) const noexcept {
    return m_controlBlock == nullptr;
  }
  bool operator!=(std::nullptr_t) const noexcept {
    return m_controlBlock != nullptr;
  }

  bool valid() const { return m_controlBlock != nullptr; }
  operator bool() const { return valid(); }

  template <typename... Args>
  static SharedBlock<T, A> make(const A& alloc, Args&&... args) {
    auto controlBlock = detail::SharedControlBlock<T, A>::make(
        alloc, std::forward<Args>(args)...);
    return SharedBlock(controlBlock);
  }

  static consteval std::size_t allocationSize()
    requires(!std::derived_from<T, FlexibleLayout>)
  {
    return sizeof(detail::SharedControlBlock<T, A>);
  }

  static consteval std::size_t allocationAlignment()
    requires(!std::derived_from<T, FlexibleLayout>)
  {
    return alignof(detail::SharedControlBlock<T, A>);
  }

 private:
  explicit SharedBlock(detail::SharedControlBlock<T, A>* controlBlock)
      : m_controlBlock(controlBlock) {}
  void release() {
    auto* old = std::exchange(m_controlBlock, nullptr);
    if (old != nullptr && old->header.useCounter.dec()) {
      detail::SharedControlBlock<T, A>::free(old);
    }
  }

  detail::SharedControlBlock<T, A>* m_controlBlock;
};

template <typename T, typename A, typename... Args>
  requires(std::is_final_v<T> && std::constructible_from<T, Args...> ||
           (std::constructible_from<T, TailAllocator&, Args...> &&
            std::derived_from<T, FlexibleLayout>))
static SharedBlock<T, A> makeSharedBlock(const A& alloc, Args&&... args) {
  return SharedBlock<T, A>::make(alloc, std::forward<Args>(args)...);
}

}  // namespace strobe
