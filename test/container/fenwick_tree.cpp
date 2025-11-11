#include "container/fenwick_tree.hpp"
#include <gtest/gtest.h>
#include <numeric>
#include <random>
#include <vector>

// ------------------------------------------------------
// 1. Basic construction and full-range sum
// ------------------------------------------------------
TEST(container_fenwick_tree, simple) {
  FenwickTree<int> tree(10);
  for (int i = 0; i < 10; ++i)
    tree.update(i, i + 1); // [1,2,...,10]

  EXPECT_EQ(tree.prefix_query(9), 55);
  EXPECT_EQ(tree.range_query(0, 9), 55);
  EXPECT_EQ(tree.range_query(3, 6), 4 + 5 + 6 + 7);
  EXPECT_EQ(tree.at(0), 1);
  EXPECT_EQ(tree.at(9), 10);
}

// ------------------------------------------------------
// 2. Verify set() updates correctly
// ------------------------------------------------------
TEST(container_fenwick_tree, set_and_update) {
  FenwickTree<int> tree(5);
  for (int i = 0; i < 5; ++i)
    tree.set(i, i + 1); // [1,2,3,4,5]

  EXPECT_EQ(tree.range_query(0, 4), 15);
  EXPECT_EQ(tree.at(2), 3);

  // Increase element 2 by 5 using update
  tree.update(2, 5);
  EXPECT_EQ(tree.at(2), 8);

  // Set element 4 to 100
  tree.set(4, 100);
  EXPECT_EQ(tree.at(4), 100);
  EXPECT_EQ(tree.range_query(0, 4), 1 + 2 + 8 + 4 + 100);
}

// ------------------------------------------------------
// 3. Randomized consistency test
// ------------------------------------------------------
TEST(container_fenwick_tree, randomized_consistency) {
  constexpr int N = 200;
  constexpr int Q = 1000;

  std::mt19937 rng(12345);
  std::uniform_int_distribution<int> dist_idx(0, N - 1);
  std::uniform_int_distribution<int> dist_val(-100, 100);

  FenwickTree<long long> tree(N);
  std::vector<long long> ref(N, 0);

  for (int q = 0; q < Q; ++q) {
    int op = rng() % 3;
    if (op == 0) {
      // update
      int i = dist_idx(rng);
      int d = dist_val(rng);
      tree.update(i, d);
      ref[i] += d;
    } else if (op == 1) {
      // set
      int i = dist_idx(rng);
      int v = dist_val(rng);
      tree.set(i, v);
      ref[i] = v;
    } else {
      // query
      int l = dist_idx(rng);
      int r = dist_idx(rng);
      if (l > r)
        std::swap(l, r);
      long long sum_ref =
          std::accumulate(ref.begin() + l, ref.begin() + r + 1, 0LL);
      EXPECT_EQ(tree.range_query(l, r), sum_ref);
    }
  }
}

TEST(container_fenwick_tree, trivial_sizes) {
  // Empty tree
  FenwickTree<int> empty(0);
  EXPECT_EQ(empty.size(), 0u);

  // Single element
  FenwickTree<int> one(1);
  EXPECT_EQ(one.at(0), 0);
  one.update(0, 42);
  EXPECT_EQ(one.at(0), 42);
  EXPECT_EQ(one.range_query(0, 0), 42);
  one.set(0, 7);
  EXPECT_EQ(one.at(0), 7);
}

TEST(container_fenwick_tree, uniform_updates) {
  constexpr int N = 1000;
  FenwickTree<long long> fw(N);

  for (int i = 0; i < N; ++i)
    fw.update(i, 1);

  EXPECT_EQ(fw.prefix_query(N - 1), N);

  // Repeatedly add 1 again
  for (int i = 0; i < N; ++i)
    fw.update(i, 1);
  EXPECT_EQ(fw.prefix_query(N - 1), 2 * N);
}

TEST(container_fenwick_tree, prefix_correctness) {
  constexpr int N = 128;
  FenwickTree<int> fw(N);
  std::vector<int> ref(N, 0);

  for (int i = 0; i < N; ++i) {
    fw.update(i, i + 1);
    ref[i] += i + 1;
  }

  for (int i = 0; i < N; ++i) {
    int expected = std::accumulate(ref.begin(), ref.begin() + i + 1, 0);
    EXPECT_EQ(fw.prefix_query(i), expected);
  }
}

TEST(container_fenwick_tree, negatives_and_inverse) {
  FenwickTree<int> fw(5);
  fw.set(0, -5);
  fw.set(1, 3);
  fw.set(2, -2);
  fw.set(3, 4);
  fw.set(4, -1);

  EXPECT_EQ(fw.prefix_query(4), -1);
  EXPECT_EQ(fw.range_query(1, 3), 3 - 2 + 4);
  EXPECT_EQ(fw.at(0), -5);
  EXPECT_EQ(fw.at(4), -1);
}

TEST(container_fenwick_tree, xor_operator) {
  FenwickTree<int, std::bit_xor<int>, std::bit_xor<int>> fxor(8);
  fxor.update(1, 0b1010);
  fxor.update(3, 0b0101);
  EXPECT_EQ(fxor.prefix_query(3), 0b1111);
  EXPECT_EQ(fxor.range_query(1, 3), 0b1111);
}

TEST(container_fenwick_tree, range_boundaries) {
  FenwickTree<int> fw(5);
  for (int i = 0; i < 5; ++i)
    fw.update(i, i + 1); // [1,2,3,4,5]

  // single elements
  for (int i = 0; i < 5; ++i)
    EXPECT_EQ(fw.range_query(i, i), i + 1);

  // prefix up to r
  EXPECT_EQ(fw.prefix_query(0), 1);
  EXPECT_EQ(fw.prefix_query(4), 15);

  // sub-ranges
  EXPECT_EQ(fw.range_query(0, 4), 15);
  EXPECT_EQ(fw.range_query(1, 3), 2 + 3 + 4);
  EXPECT_EQ(fw.range_query(2, 4), 3 + 4 + 5);
}

TEST(container_fenwick_tree, set_overwrite) {
  FenwickTree<int> fw(6);
  for (int i = 0; i < 6; ++i)
    fw.set(i, i + 1); // [1,2,3,4,5,6]

  EXPECT_EQ(fw.range_query(0, 5), 21);

  // overwrite
  fw.set(2, 10); // [1,2,10,4,5,6]
  EXPECT_EQ(fw.at(2), 10);
  EXPECT_EQ(fw.range_query(0, 5), 28);

  // reduce value
  fw.set(4, 0); // [1,2,10,4,0,6]
  EXPECT_EQ(fw.range_query(0, 5), 23);
}

TEST(container_fenwick_tree, large_uniform_adds) {
  constexpr int N = 100000;
  FenwickTree<long long> fw(N);
  for (int i = 0; i < N; ++i)
    fw.update(i, 1);

  EXPECT_EQ(fw.prefix_query(N - 1), N);
  EXPECT_EQ(fw.range_query(1000, 1999), 1000);
}

TEST(container_fenwick_tree, xor_semantics) {
  FenwickTree<int, std::bit_xor<int>, std::bit_xor<int>> fxor(8);
  fxor.update(1, 0b0101);
  fxor.update(3, 0b1100);
  fxor.update(4, 0b0101);

  EXPECT_EQ(fxor.prefix_query(4), 0b1100);
  EXPECT_EQ(fxor.range_query(1, 3), 0b1001);
}

TEST(container_fenwick_tree, negatives_mixed_signs) {
  FenwickTree<int> fw(6);
  std::vector<int> vals = {5, -3, 7, -2, 4, -1};
  for (int i = 0; i < 6; ++i)
    fw.set(i, vals[i]);

  EXPECT_EQ(fw.prefix_query(5), std::accumulate(vals.begin(), vals.end(), 0));
  EXPECT_EQ(fw.range_query(1, 3), (-3 + 7 - 2));
}

TEST(container_fenwick_tree, random_regression) {
  constexpr int N = 300;
  constexpr int Q = 2000;
  FenwickTree<long long> fw(N);
  std::vector<long long> ref(N, 0);
  std::mt19937 rng(98765);
  std::uniform_int_distribution<int> di(0, N - 1);
  std::uniform_int_distribution<int> dv(-50, 50);

  for (int q = 0; q < Q; ++q) {
    int op = rng() % 3;
    if (op == 0) {
      int i = di(rng), d = dv(rng);
      fw.update(i, d);
      ref[i] += d;
    } else if (op == 1) {
      int i = di(rng), v = dv(rng);
      fw.set(i, v);
      ref[i] = v;
    } else {
      int l = di(rng), r = di(rng);
      if (l > r)
        std::swap(l, r);
      long long expected =
          std::accumulate(ref.begin() + l, ref.begin() + r + 1, 0LL);
      EXPECT_EQ(fw.range_query(l, r), expected);
    }
  }
}
