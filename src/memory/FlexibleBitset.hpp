/* #pragma once */
/*  */
/* #include "memory/FlexibleLayout.hpp" */
/* namespace strobe { */
/*  */
/* class FlexibleBitset : public FlexibleLayout { */
/*   using word = unsigned long; */
/*  */
/* public: */
/*   FlexibleBitset(TailAllocator &tailAllocator, std::size_t n) */
/*       : FlexibleLayout(tailAllocator) { */
/*     std::size_t amountOfWords = (n + sizeof(word) - 1) / sizeof(word); */
/*     TailView<word> v = tailAllocator.reserve<word>(amountOfWords); */
/*     void *block = tailAllocator.allocate(); */
/*     m_bitset = v.get(block); */
/*   } */
/*  */
/*   bool get(std::size_t x) const { */
/*     std::size_t wordIdx = (x / sizeof(unsigned long)); */
/*     std::size_t bitIdx = x & (sizeof(unsigned long) - 1); */
/*     return (m_bitset[wordIdx] & (1 << bitIdx)) != 0; */
/*   } */
/*  */
/*   void set(std::size_t x) { */
/*     std::size_t wordIdx = (x / sizeof(unsigned long)); */
/*     std::size_t bitIdx = x & (sizeof(unsigned long) - 1); */
/*     m_bitset[wordIdx] |= (1 << bitIdx); */
/*   } */
/*  */
/*   void clear(std::size_t x) { */
/*     std::size_t wordIdx = (x / sizeof(unsigned long)); */
/*     std::size_t bitIdx = x & (sizeof(unsigned long) - 1); */
/*     m_bitset[wordIdx] &= ~(1 << bitIdx); */
/*   } */
/*  */
/* private: */
/*   word *m_bitset; */
/* }; */
/* } // namespace strobe */
