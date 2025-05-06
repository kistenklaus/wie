#include "memory/BuddyAllocator.hpp"
#include <algorithm>
#include <benchmark/benchmark.h>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <random>

template <typename Allocator>
static void BM_pool_random_order(benchmark::State &state, Allocator alloc) {
  std::size_t blockSize = state.range(0);
  std::size_t blockCount = state.range(1);

  std::vector<void *> ptrs(blockCount);

  std::vector<std::size_t> idx(blockCount * 2);
  for (std::size_t i = 0; i < blockCount; ++i) {
    std::size_t j = 2 * i;
    idx[j] = i;
    idx[j + 1] = i;
  }

  std::mt19937 rng;
  for (auto _ : state) {
    state.PauseTiming();
    std::memset(ptrs.data(), 0, ptrs.size() * sizeof(void *));
    std::ranges::shuffle(idx, std::default_random_engine(rng()));
    state.ResumeTiming();
    for (auto i : idx) {
      auto &ptr = ptrs[i];
      if (ptr == nullptr) {
        ptr = alloc.allocate(blockSize, blockSize);
        assert(ptr);
        assert(ptrs[i]);
      } else {
        alloc.deallocate(ptr, blockSize, blockSize);
        ptr = nullptr;
        assert(!ptr);
        assert(!ptrs[i]);
      }
    }
  }
}

//BENCHMARK_CAPTURE(BM_pool_random_order<strobe::Mallocator>, malloc,
//                  strobe::Mallocator{})
//    ->Args({8, 1 << 10})
//    ->Args({16, 1 << 10})
//    ->Args({64, 1 << 10})
//    ->Args({256, 1 << 10})
//    ->Args({1024, 1 << 10})
//    ->Args({4096, 1 << 10})
//    ->Args({8192, 1 << 10});

using Buddy8 = strobe::BuddyAllocator<(1ull << 10) * 8, 8>;
BENCHMARK_CAPTURE(BM_pool_random_order<Buddy8>, buddy, Buddy8{})
  ->Args({8, 1 << 10});

using Buddy16 = strobe::BuddyAllocator<(1ull << 10) * 16, 16>;
BENCHMARK_CAPTURE(BM_pool_random_order<Buddy16>, buddy, Buddy16{})
  ->Args({16, 1 << 10});

using Buddy64 = strobe::BuddyAllocator<(1ull << 10) * 64, 64>;
BENCHMARK_CAPTURE(BM_pool_random_order<Buddy64>, buddy, Buddy64{})
  ->Args({64, 1 << 10});

using Buddy256 = strobe::BuddyAllocator<(1ull << 10) * 256, 256>;
BENCHMARK_CAPTURE(BM_pool_random_order<Buddy256>, buddy, Buddy256{})
  ->Args({256, 1 << 10});

using Buddy1024 = strobe::BuddyAllocator<(1ull << 10) * 1024, 1024>;
BENCHMARK_CAPTURE(BM_pool_random_order<Buddy1024>, buddy, Buddy1024{})
  ->Args({1024, 1 << 10});

using Buddy4096 = strobe::BuddyAllocator<(1ull << 10) * 4096, 4096>;
BENCHMARK_CAPTURE(BM_pool_random_order<Buddy4096>, buddy, Buddy4096{})
  ->Args({4096, 1 << 10});

using Buddy2048 = strobe::BuddyAllocator<(1ull << 10) * 4096, 2048>;
BENCHMARK_CAPTURE(BM_pool_random_order<Buddy2048>, buddy, Buddy2048{})
  ->Args({4096, 1 << 10});

using Buddy8192 = strobe::BuddyAllocator<(1ull << 10) * 8192, 8192>;
BENCHMARK_CAPTURE(BM_pool_random_order<Buddy8192>, buddy, Buddy8192{})
  ->Args({8192, 1 << 10});


//BENCHMARK_CAPTURE(BM_pool_random_order<strobe::PageAllocator>, mmap,
//                  strobe::PageAllocator{})
//  ->Args({4096, 1 << 10})
//  ->Args({8192, 1 << 10});

