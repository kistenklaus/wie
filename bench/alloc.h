#include <benchmark/benchmark.h>
#include <cstdlib>

constexpr size_t ALLOC_COUNT = 10000;

/*static void BM_alloc_malloc(benchmark::State &state) {*/
/*  auto size = state.range(0);*/
/*  std::vector<void *> mem;*/
/*  mem.reserve(ALLOC_COUNT);*/
/**/
/**/
/*  auto rng = std::default_random_engine {};*/
/*  for (auto _ : state) {*/
/*      for (size_t i = 0; i < ALLOC_COUNT; ++i) {*/
/*        mem.push_back(malloc(sizeof(int)));*/
/*      }*/
/*      state.PauseTiming();*/
/*      std::shuffle(mem.begin(), mem.end(), rng);*/
/*      state.ResumeTiming();*/
/**/
/*      for (size_t i = 0; i < ALLOC_COUNT / 2; ++i) {*/
/*        auto p = mem.back();*/
/*        mem.pop_back();*/
/*        free(p);*/
/*      }*/
/**/
/*      for (size_t i = 0; i < ALLOC_COUNT / 2; ++i) {*/
/*        mem.push_back(malloc(sizeof(int)));*/
/*      }*/
/**/
/*      for (auto p : mem) {*/
/*        free(p);*/
/*      }*/
/*      mem.clear();*/
/*  }*/
/*}*/
/**/
/*template <size_t ELEM_SIZE> static void BM_alloc_pool(benchmark::State &state) {*/
/*  auto size = state.range(0);*/
/**/
/*  constexpr auto CHUNK_SIZE = std::max((size_t)1, 4096 / ELEM_SIZE * 8);*/
/**/
/*  using CountingAllocator = wie::CountingAllocator<std::allocator<*/
/*      wie::internal::LinkedPoolBucketChunk<ELEM_SIZE, CHUNK_SIZE>>>;*/
/**/
/*  CountingAllocator countingAllocator{};*/
/*  {*/
/*    wie::FreelistPool<ELEM_SIZE, CHUNK_SIZE, CountingAllocator> pool{*/
/*        countingAllocator};*/
/**/
/*    std::vector<void *> mem;*/
/*    mem.reserve(ALLOC_COUNT);*/
/*    auto rng = std::default_random_engine {};*/
/*    for (auto _ : state) {*/
/*      for (size_t i = 0; i < ALLOC_COUNT; ++i) {*/
/*        mem.push_back(pool.allocate());*/
/*      }*/
/*      state.PauseTiming();*/
/*      std::shuffle(mem.begin(), mem.end(), rng);*/
/*      state.ResumeTiming();*/
/**/
/*      for (size_t i = 0; i < ALLOC_COUNT / 2; ++i) {*/
/*        auto p = mem.back();*/
/*        mem.pop_back();*/
/*        pool.deallocate(p);*/
/*      }*/
/**/
/*      for (size_t i = 0; i < ALLOC_COUNT / 2; ++i) {*/
/*        mem.push_back(pool.allocate());*/
/*      }*/
/**/
/*      for (auto p : mem) {*/
/*        pool.deallocate(p);*/
/*      }*/
/**/
/**/
/*      mem.clear();*/
/*    }*/
/*  }*/
/*  state.counters["Alloc"] = countingAllocator.allocCount();*/
/*}*/
/**/
/*BENCHMARK(BM_alloc_malloc)->Arg(1);*/
/*BENCHMARK(BM_alloc_pool<1>);*/
/*BENCHMARK(BM_alloc_malloc)->Arg(4);*/
/*BENCHMARK(BM_alloc_pool<4>);*/
/*BENCHMARK(BM_alloc_malloc)->Arg(8);*/
/*BENCHMARK(BM_alloc_pool<8>);*/
/*BENCHMARK(BM_alloc_malloc)->Arg(16);*/
/*BENCHMARK(BM_alloc_pool<16>);*/
/*BENCHMARK(BM_alloc_malloc)->Arg(32);*/
/*BENCHMARK(BM_alloc_pool<32>);*/
/*BENCHMARK(BM_alloc_malloc)->Arg(64);*/
/*BENCHMARK(BM_alloc_pool<64>);*/
/*BENCHMARK(BM_alloc_malloc)->Arg(128);*/
/*BENCHMARK(BM_alloc_pool<128>);*/
/*BENCHMARK(BM_alloc_malloc)->Arg(4096);*/
/*BENCHMARK(BM_alloc_pool<4096>);*/
/*BENCHMARK(BM_alloc_malloc)->Arg(8192);*/
/*BENCHMARK(BM_alloc_pool<8192>);*/
