#include "./pages.hpp"

std::size_t strobe::page_size() {
  static const std::size_t size =
      static_cast<std::size_t>(sysconf(_SC_PAGESIZE));
  return size;
}

