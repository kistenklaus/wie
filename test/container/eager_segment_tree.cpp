#include "container/eager_segment_tree.hpp"
#include <gtest/gtest.h>

#include <numeric>
#include <random>
#include <string>
#include <vector>

// Basic construction and full-range sum
TEST(container_eager_segment_tree, full_range_sum) {
  std::vector<int> values = {1, 2, 3, 4};
  using Op = std::plus<int>;
  EagerSegmentTree<int, Op> segment(values.begin(), values.end());

  int agg = segment.range_query(/* inclusive */ 0, /* exclusive */ 4);
  EXPECT_EQ(agg, 1 + 2 + 3 + 4);

  // Check that at() matches underlying values
  for (std::size_t i = 0; i < values.size(); ++i) {
    EXPECT_EQ(segment.at(i), values[i]);
  }
}

// Single-element queries should match the stored values
TEST(container_eager_segment_tree, single_element_queries) {
  std::vector<int> values = {5, -1, 7, 3, 8};
  using Op = std::plus<int>;
  EagerSegmentTree<int, Op> segment(values.begin(), values.end());

  for (std::size_t i = 0; i < values.size(); ++i) {
    EXPECT_EQ(segment.range_query(i, i + 1), values[i]);
    EXPECT_EQ(segment.at(i), values[i]);
  }
}

// Range update with Op = plus<int>: v = v + delta on [l, r)
TEST(container_eager_segment_tree, range_update_additive) {
  std::vector<int> values = {1, 2, 3, 4, 5};
  using Op = std::plus<int>;
  EagerSegmentTree<int, Op> segment(values.begin(), values.end());

  // Add 10 to indices [1, 4) i.e. 1,2,3
  segment.range_update(/* inclusive */ 1, /* exclusive */ 4, /* delta */ 10);
  for (std::size_t i = 1; i < 4; ++i) {
    values[i] += 10;
  }

  // Check at()
  for (std::size_t i = 0; i < values.size(); ++i) {
    EXPECT_EQ(segment.at(i), values[i]);
  }

  // Check some range queries
  EXPECT_EQ(segment.range_query(0, 5),
            std::accumulate(values.begin(), values.end(), 0));
  EXPECT_EQ(segment.range_query(1, 4), values[1] + values[2] + values[3]);
  EXPECT_EQ(segment.range_query(2, 3), values[2]);
}

// Point update and set()
TEST(container_eager_segment_tree, point_update_and_set) {
  std::vector<int> values = {10, 20, 30, 40};
  using Op = std::plus<int>;
  EagerSegmentTree<int, Op> segment(values.begin(), values.end());

  // v[2] += 5
  segment.update(/* index */ 2, /* delta */ 5);
  values[2] += 5;

  EXPECT_EQ(segment.at(2), values[2]);
  EXPECT_EQ(segment.range_query(0, 4),
            std::accumulate(values.begin(), values.end(), 0));

  // set index 0 to 100
  segment.set(/* index */ 0, /* new_value */ 100);
  values[0] = 100;

  EXPECT_EQ(segment.at(0), 100);
  EXPECT_EQ(segment.range_query(0, 1), 100);
  EXPECT_EQ(segment.range_query(0, 4),
            std::accumulate(values.begin(), values.end(), 0));
}

// lhs_update should apply Op(delta, v) instead of Op(v, delta).
// For std::plus<std::string>, this is prepend vs append.
TEST(container_eager_segment_tree, lhs_update_string_prepend) {
  std::vector<std::string> values = {"a", "b", "c"};
  using Op = std::plus<std::string>;
  EagerSegmentTree<std::string, Op> segment(values.begin(), values.end());

  // v[1] = v[1] + "X" -> "bX"
  segment.update(/* index */ 1, /* delta */ std::string("X"));
  values[1] = values[1] + "X";
  EXPECT_EQ(segment.at(1), values[1]);

  // v[1] = "Y" + v[1] via lhs_update -> "YbX"
  segment.lhs_update(/* index */ 1, /* delta */ std::string("Y"));
  values[1] = "Y" + values[1];
  EXPECT_EQ(segment.at(1), values[1]);

  // Range query should concatenate in order
  std::string expected_concat = values[0] + values[1] + values[2];
  EXPECT_EQ(segment.range_query(0, 3), expected_concat);
}

// Mixed range and point updates, including lhs_update
TEST(container_eager_segment_tree, mixed_updates_and_queries) {
  std::vector<int> values = {0, 1, 2, 3, 4, 5, 6, 7};
  using Op = std::plus<int>;
  EagerSegmentTree<int, Op> segment(values.begin(), values.end());

  // 1) range_update [0, 8) with +1
  segment.range_update(0, 8, 1);
  for (int& v : values) v += 1;

  // 2) point update at index 3: +4
  segment.update(3, 4);
  values[3] += 4;

  // 3) lhs_update at index 5 with +10; for ints with plus this is same as update
  segment.lhs_update(5, 10);
  values[5] = 10 + values[5]; // = values[5] + 10, but we follow the semantic

  // 4) set index 2
  segment.set(2, 42);
  values[2] = 42;

  // Check a variety of ranges
  EXPECT_EQ(segment.range_query(0, 8),
            std::accumulate(values.begin(), values.end(), 0));

  EXPECT_EQ(segment.range_query(0, 4),
            values[0] + values[1] + values[2] + values[3]);

  EXPECT_EQ(segment.range_query(4, 7),
            values[4] + values[5] + values[6]);

  // Per-element check via at()
  for (std::size_t i = 0; i < values.size(); ++i) {
    EXPECT_EQ(segment.at(i), values[i]) << "Mismatch at index " << i;
  }
}

// Empty range update should be a no-op (if your implementation allows it)
TEST(container_eager_segment_tree, empty_range_update_noop) {
  std::vector<int> values = {1, 2, 3};
  using Op = std::plus<int>;
  EagerSegmentTree<int, Op> segment(values.begin(), values.end());

  // [1,1) is an empty range
  segment.range_update(1, 1, 5);

  // Values should remain unchanged
  for (std::size_t i = 0; i < values.size(); ++i) {
    EXPECT_EQ(segment.at(i), values[i]);
  }

  EXPECT_EQ(segment.range_query(0, 3),
            std::accumulate(values.begin(), values.end(), 0));
}

// Randomized test vs. a naive vector implementation
TEST(container_eager_segment_tree, randomized_against_naive) {
  using Op = std::plus<int>;
  constexpr int N   = 32;
  constexpr int OPS = 500;

  std::vector<int> values(N, 0);
  EagerSegmentTree<int, Op> segment(values.begin(), values.end());

  std::mt19937 rng(123456);
  std::uniform_int_distribution<int> idx_dist(0, N - 1);
  std::uniform_int_distribution<int> delta_dist(-5, 5);
  std::uniform_int_distribution<int> op_dist(0, 2); // 0: range_update, 1: update, 2: query

  for (int step = 0; step < OPS; ++step) {
    int op = op_dist(rng);

    if (op == 0) {
      // range_update on [l, r]
      int l = idx_dist(rng);
      int r = idx_dist(rng);
      if (l > r) std::swap(l, r);
      if (l == r) continue; // skip empty ranges here

      int delta = delta_dist(rng);
      segment.range_update(l, r + 1, delta);
      for (int i = l; i <= r; ++i) {
        values[i] += delta;
      }

    } else if (op == 1) {
      // point update
      int i = idx_dist(rng);
      int delta = delta_dist(rng);
      segment.update(i, delta);
      values[i] += delta;

    } else {
      // range_query, compare to naive sum
      int l = idx_dist(rng);
      int r = idx_dist(rng);
      if (l > r) std::swap(l, r);
      r += 1; // exclusive

      int expected = 0;
      for (int i = l; i < r; ++i) {
        expected += values[i];
      }

      EXPECT_EQ(segment.range_query(l, r), expected)
          << "Mismatch in range_query(" << l << ", " << r << ") at step "
          << step;
    }
  }
}
