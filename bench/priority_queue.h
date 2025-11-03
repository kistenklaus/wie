#pragma once
#include "benchmark/benchmark.h"
#include "container/binary_heap.hpp"
#include "container/kary_heap.hpp"
#include <iostream>
#include <queue>
#include <random>

static void BM_BinaryHeapInsert(benchmark::State &state) {
  std::mt19937 prng(0);
  std::uniform_int_distribution<int> dist;

  std::size_t COUNT = state.range(0);

  std::vector<int> values(COUNT);
  for (auto &v : values) {
    v = dist(prng);
  }

  // Perform setup here
  for (auto _ : state) {
    state.PauseTiming();
    strobe::BinaryHeap<int> heap;
    heap.reserve(COUNT);
    state.ResumeTiming();
    for (const auto &v : values) {
      heap.push(v);
    }
    benchmark::DoNotOptimize(heap);
  }
}

static void BM_KaryHeapInsert(benchmark::State &state) {
  std::mt19937 prng(0);
  std::uniform_int_distribution<int> dist;

  std::size_t COUNT = state.range(0);

  std::vector<int> values(COUNT);
  for (auto &v : values) {
    v = dist(prng);
  }

  // Perform setup here
  for (auto _ : state) {
    state.PauseTiming();
    strobe::KAryHeap<int, 4> heap;
    heap.reserve(COUNT);
    state.ResumeTiming();

    for (const auto &v : values) {
      heap.push(v);
    }
    benchmark::DoNotOptimize(heap);
  }
}

static void BM_BinaryHeapRandomizedInsertRemove(benchmark::State &state) {
  std::mt19937 prng(0);
  std::uniform_int_distribution<int> dist(1);
  std::uniform_int_distribution<int> choice(0, 1);

  std::size_t COUNT = state.range(0);

  std::vector<int> values(COUNT);
  for (auto &v : values) {
    if (choice(prng)) {
      v = 0;
    } else {
      v = dist(prng);
    }
  }

  // Perform setup here
  for (auto _ : state) {
    state.PauseTiming();
    strobe::BinaryHeap<int> heap;
    heap.reserve(COUNT);
    state.ResumeTiming();

    for (const auto &v : values) {
      if (v == 0 && !heap.empty()) {
        heap.pop();
      } else {
        heap.push(v);
      }
    }
    benchmark::DoNotOptimize(heap);
  }
}

static void BM_Kary2HeapRandomizedInsertRemove(benchmark::State &state) {
  std::mt19937 prng(0);
  std::uniform_int_distribution<int> dist(1);
  std::uniform_int_distribution<int> choice(0, 1);

  std::size_t COUNT = state.range(0);

  std::vector<int> values(COUNT);
  for (auto &v : values) {
    if (choice(prng)) {
      v = 0;
    } else {
      v = dist(prng);
    }
  }

  // Perform setup here
  for (auto _ : state) {
    // state.PauseTiming();
    strobe::KAryHeap<int, 4> heap;
    // heap.reserve(COUNT);
    // state.ResumeTiming();

    for (const auto &v : values) {
      if (v == 0 && !heap.empty()) {
        heap.pop();
      } else {
        heap.push(v);
      }
    }
    benchmark::DoNotOptimize(heap);
  }
}

static void BM_Kary4HeapRandomizedInsertRemove(benchmark::State &state) {
  std::mt19937 prng(0);
  std::uniform_int_distribution<int> dist(1);
  std::uniform_int_distribution<int> choice(0, 1);

  std::size_t COUNT = state.range(0);

  std::vector<int> values(COUNT);
  for (auto &v : values) {
    if (choice(prng)) {
      v = 0;
    } else {
      v = dist(prng);
    }
  }

  // Perform setup here
  for (auto _ : state) {
    // state.PauseTiming();
    strobe::KAryHeap<int, 4> heap;
    // heap.reserve(COUNT);
    // state.ResumeTiming();

    for (const auto &v : values) {
      if (v == 0 && !heap.empty()) {
        heap.pop();
      } else {
        heap.push(v);
      }
    }
    benchmark::DoNotOptimize(heap);
  }
}

static void BM_Kary8HeapRandomizedInsertRemove(benchmark::State &state) {
  std::mt19937 prng(0);
  std::uniform_int_distribution<int> dist(1);
  std::uniform_int_distribution<int> choice(0, 1);

  std::size_t COUNT = state.range(0);

  std::vector<int> values(COUNT);
  for (auto &v : values) {
    if (choice(prng)) {
      v = 0;
    } else {
      v = dist(prng);
    }
  }

  // Perform setup here
  for (auto _ : state) {
    // state.PauseTiming();
    strobe::KAryHeap<int, 4> heap;
    // heap.reserve(COUNT);
    // state.ResumeTiming();

    for (const auto &v : values) {
      if (v == 0 && !heap.empty()) {
        heap.pop();
      } else {
        heap.push(v);
      }
    }
    benchmark::DoNotOptimize(heap);
  }
}

static void BM_StdPriorityQueueRandomizedInsertRemove(benchmark::State &state) {
  std::mt19937 prng(0);
  std::uniform_int_distribution<int> dist(1);
  std::uniform_int_distribution<int> choice(0, 1);

  std::size_t COUNT = state.range(0);

  std::vector<int> values(COUNT);
  for (auto &v : values) {
    if (choice(prng)) {
      v = 0;
    } else {
      v = dist(prng);
    }
  }

  // Perform setup here
  for (auto _ : state) {
    // state.PauseTiming();
    std::priority_queue<int> heap;
    // state.ResumeTiming();

    for (const auto &v : values) {
      if (v == 0 && !heap.empty()) {
        heap.pop();
      } else {
        heap.push(v);
      }
    }
    benchmark::DoNotOptimize(heap);
  }
}

BENCHMARK(BM_BinaryHeapInsert) //
    ->Arg(1000)
    ->Arg(10000)
    ->Arg(100000)
    ->Arg(1000000);
BENCHMARK(BM_KaryHeapInsert) //
    ->Arg(1000)
    ->Arg(10000)
    ->Arg(100000)
    ->Arg(1000000);

BENCHMARK(BM_BinaryHeapRandomizedInsertRemove) //
    ->Arg(1000)
    ->Arg(10000)
    ->Arg(100000)
    ->Arg(1000000);


BENCHMARK(BM_Kary2HeapRandomizedInsertRemove) //
    ->Arg(1000)
    ->Arg(10000)
    ->Arg(100000)
    ->Arg(1000000);

BENCHMARK(BM_Kary4HeapRandomizedInsertRemove) //
    ->Arg(1000)
    ->Arg(10000)
    ->Arg(100000)
    ->Arg(1000000);

BENCHMARK(BM_Kary8HeapRandomizedInsertRemove) //
    ->Arg(1000)
    ->Arg(10000)
    ->Arg(100000)
    ->Arg(1000000);

BENCHMARK(BM_StdPriorityQueueRandomizedInsertRemove) //
    ->Arg(1000)
    ->Arg(10000)
    ->Arg(100000)
    ->Arg(1000000);
