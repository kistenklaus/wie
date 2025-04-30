#include "memory/FlexibleLayout.hpp"
#include "memory/Mallocator.hpp"
#include "memory/PolyAllocator.hpp"
#include <gtest/gtest.h>
#include <initializer_list>
#include <print>

struct Header {
  std::size_t x;
  std::size_t y;
  std::size_t n;
  std::size_t *fam;

  static Header *make(strobe::TailAllocator &alloc) {
    /* auto view = alloc.reserve<std::size_t>(values.size()); */
    /* Header* self = reinterpret_cast<Header*>(alloc.allocate()); */
    /* self->x = x; */
    /* self->y = y; */
    /* self->n = 0; */
    /* for (auto v : values) { */
    /*   self->fam[self->n] = v; */
    /*   self->n += 1; */
    /* } */
    return nullptr;
  }

private:
  Header(std::size_t x, std::size_t y, std::size_t *fam)
      : x(x), y(y), fam(fam) {}
};

static_assert(strobe::FlexibleLayout<Header>);

TEST(TailAllocator, simple) {

  strobe::Mallocator mallocator;
  strobe::PolyAllocator poly{&mallocator};
  strobe::TailAllocator tail{std::move(poly), sizeof(Header), alignof(Header)};


  /* std::size_t n = 10; */
  /* Header *header = Header::make(tail, 1, 2, {10,10}); */

  /* header->x = 1; */
  /* header->y = 2; */


}
