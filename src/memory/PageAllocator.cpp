#include "./PageAllocator.hpp"

#include <cerrno>
#include <print>
#include <sys/mman.h>
#include <unistd.h>

#include <algorithm>
#include <cstddef>

#include "memory/align.hpp"
#include "memory/pages.hpp"
namespace strobe {

#ifdef NDEBUG
static constexpr bool USE_GUARD_PAGES = false;
#else
static constexpr bool USE_GUARD_PAGES = false;
#endif

void* PageAllocator::allocate(std::size_t size, std::size_t alignment) {
  if (size == 0) {
    return nullptr;
  }
  const std::size_t page = page_size();
  size = align_up(size, page);

  std::size_t totalSize = size;
  if (USE_GUARD_PAGES) {
    totalSize += 2 * page;
  }

  void* raw = mmap(nullptr, totalSize, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  if (raw == MAP_FAILED) return nullptr;

  if (USE_GUARD_PAGES) {
    // Protect first and last page
    mprotect(raw, page, PROT_NONE);
    mprotect(static_cast<char*>(raw) + page + size, page, PROT_NONE);
    return static_cast<char*>(raw) + page;
  }

  return raw;
}

void PageAllocator::deallocate(void* ptr, std::size_t size,
                               std::size_t alignment) {
  if (!ptr) return;

  const std::size_t page = page_size();
  alignment = std::max(alignment, page);
  size = align_up(size, page);

  void* raw = ptr;
  std::size_t totalSize = size;

  if (USE_GUARD_PAGES) {
    raw = static_cast<char*>(ptr) - page;
    totalSize += 2 * page;
  }

  int result = munmap(raw, totalSize);
  if (result != 0) {
    std::println("errno = {}", errno);
  }
  assert(result == 0 && "munmap failed");
}

}  // namespace strobe
