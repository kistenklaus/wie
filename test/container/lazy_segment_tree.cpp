#include "container/lazy_segment_tree.hpp"
#include <gtest/gtest.h>

#include <numeric>
#include <random>
#include <vector>

using Op = std::plus<int>;

static auto apply_add = [](int& v, size_t len, int tag) {
  v =  v + tag * static_cast<int>(len);
};

TEST(container_lazy_segment_tree, construct_and_query_full_range) {
  std::vector<int> values = {1, 2, 3, 4};
  LazySegmentTree<int, int, Op, Op, decltype(apply_add)> seg{
      values.begin(), values.end(), Op{}, Op{}, apply_add};

  EXPECT_EQ(seg.range_query(0, 4), 10);
}

TEST(container_lazy_segment_tree, range_update_affects_sum) {
  std::vector<int> values = {1, 2, 3, 4};
  LazySegmentTree<int, int, Op, Op, decltype(apply_add)> seg{
      values.begin(), values.end(), Op{}, Op{}, apply_add};

  // add 1 to [1, 3)
  seg.range_update(1, 3, 1);
  // expected array: {1, 3, 4, 4}
  EXPECT_EQ(seg.range_query(0, 4), 12);
  EXPECT_EQ(seg.range_query(0, 1), 1);
  EXPECT_EQ(seg.range_query(1, 3), 7);
  EXPECT_EQ(seg.range_query(2, 4), 8);
}

TEST(container_lazy_segment_tree, multiple_updates_accumulate) {
  std::vector<int> values = {1, 2, 3, 4};
  LazySegmentTree<int, int, Op, Op, decltype(apply_add)> seg{
      values.begin(), values.end(), Op{}, Op{}, apply_add};

  seg.range_update(0, 2, 1); // +1 to [0,2)
  seg.range_update(1, 4, 2); // +2 to [1,4)

  // final array = [2,5,5,6]
  EXPECT_EQ(seg.range_query(0, 4), 18);
  EXPECT_EQ(seg.range_query(0, 1), 2);
  EXPECT_EQ(seg.range_query(1, 2), 5);
  EXPECT_EQ(seg.range_query(3, 4), 6);
}

TEST(container_lazy_segment_tree, partial_range_sum) {
  std::vector<int> values = {1, 2, 3, 4, 5};
  LazySegmentTree<int, int, Op, Op, decltype(apply_add)> seg{
      values.begin(), values.end(), Op{}, Op{}, apply_add};

  EXPECT_EQ(seg.range_query(0, 3), 6);
  EXPECT_EQ(seg.range_query(2, 5), 12);
  EXPECT_EQ(seg.range_query(1, 4), 9);
}

TEST(container_lazy_segment_tree, random_updates_and_queries) {
  constexpr int N = 64;
  constexpr int OPS = 200;
  std::vector<int> values(N);
  std::iota(values.begin(), values.end(), 0);

  LazySegmentTree<int, int, Op, Op, decltype(apply_add)> seg{
      values.begin(), values.end(), Op{}, Op{}, apply_add};

  std::mt19937 rng(1234);
  std::uniform_int_distribution<int> dist_idx(0, N - 1);
  std::uniform_int_distribution<int> dist_val(1, 5);

  for (int t = 0; t < OPS; ++t) {
    int l = dist_idx(rng);
    int r = dist_idx(rng);
    if (l > r)
      std::swap(l, r);
    ++r; // make [l, r)
    if (r > N)
      r = N;

    if (rng() % 2 == 0) {
      int delta = dist_val(rng);
      seg.range_update(l, r, delta);
      for (int i = l; i < r; ++i)
        values[i] += delta;
    } else {
      int expected = std::accumulate(values.begin() + l, values.begin() + r, 0);
      int got = seg.range_query(l, r);
      EXPECT_EQ(got, expected) << "Mismatch at [" << l << "," << r << ")";
    }
  }
}

TEST(container_lazy_segment_tree, empty_tree_behaves_safely) {
  LazySegmentTree<int> seg(0);
  EXPECT_EQ(seg.size(), 0);
  EXPECT_EQ(seg.range_query(0, 0), 0);
  seg.range_update(0, 0, 42); // should not crash
}

TEST(container_lazy_segment_tree, single_element_tree) {
  std::vector<int> v = {5};
  LazySegmentTree<int, int, std::plus<int>> seg(v.begin(), v.end());
  EXPECT_EQ(seg.range_query(0, 1), 5);

  seg.range_update(0, 1, 3);
  // default Apply is v + tag*len = v + tag
  auto apply = [](int& v, size_t len, int tag) { v = v + tag * (int)len; };
  LazySegmentTree<int, int, std::plus<int>, std::plus<int>, decltype(apply)>
      seg2(v.begin(), v.end(), {}, {}, apply);
  seg2.range_update(0, 1, 3);
  EXPECT_EQ(seg2.range_query(0, 1), 8);
}

TEST(container_lazy_segment_tree, non_commutative_concat) {
  using T = std::string;
  auto combine = [](const T &a, const T &b) { return a + b; };
  auto compose = [](const T &a, const T &b) { return a + b; };
  auto apply = [](T &v, size_t, const T &tag) { v =  v + tag; };

  std::vector<T> init = {"A", "B", "C", "D"};
  LazySegmentTree<T, T, decltype(combine), decltype(compose), decltype(apply)>
      seg(init.begin(), init.end(), combine, compose, apply);

  EXPECT_EQ(seg.range_query(0, 4), "ABCD");

  seg.range_update(1, 3, "x"); // append x to B and C
  EXPECT_EQ(seg.range_query(0, 4), "ABxCxD");
  EXPECT_EQ(seg.range_query(1, 2), "Bx");
}

TEST(container_lazy_segment_tree, overlapping_lazy_propagation) {
  std::vector<int> v = {1, 2, 3, 4, 5};
  auto apply = [](int& v, size_t len, int tag) { v =  v + tag * (int)len; };
  LazySegmentTree<int, int, std::plus<int>, std::plus<int>, decltype(apply)>
      seg(v.begin(), v.end(), {}, {}, apply);

  seg.range_update(0, 5, 1); // +1 to all
  seg.range_update(1, 4, 2); // +2 to middle
  // final = [2,5,6,7,6]
  EXPECT_EQ(seg.range_query(0, 5), 26);
  EXPECT_EQ(seg.range_query(1, 4), 18);
  EXPECT_EQ(seg.range_query(2, 3), 6);
}

TEST(container_lazy_segment_tree, non_power_of_two_size) {
  std::vector<int> v(6);
  std::iota(v.begin(), v.end(), 1); // 1..6
  auto apply = [](int& v, size_t len, int tag) { v = v + tag * (int)len; };
  LazySegmentTree<int, int, std::plus<int>, std::plus<int>, decltype(apply)>
      seg(v.begin(), v.end(), {}, {}, apply);

  EXPECT_EQ(seg.range_query(0, 6), 21);

  seg.range_update(2, 6, 1); // +1 to last 4
  EXPECT_EQ(seg.range_query(0, 6), 25);
  EXPECT_EQ(seg.range_query(2, 4), 9);
}

TEST(container_lazy_segment_tree, alternating_updates_and_queries) {
  constexpr int N = 256;
  std::vector<int> v(N);
  std::iota(v.begin(), v.end(), 0);

  auto apply = [](int &v, size_t len, int tag) { v =  v + tag * (int)len; };
  LazySegmentTree<int, int, std::plus<int>, std::plus<int>, decltype(apply)>
      seg(v.begin(), v.end(), {}, {}, apply);

  std::mt19937 rng(123);
  std::uniform_int_distribution<int> dist(0, N - 1);
  std::uniform_int_distribution<int> delta(-3, 3);

  for (int step = 0; step < 1000; ++step) {
    int l = dist(rng);
    int r = dist(rng);
    if (l > r)
      std::swap(l, r);
    ++r;
    if (r > N)
      r = N;

    if (rng() % 2 == 0) {
      int d = delta(rng);
      seg.range_update(l, r, d);
      for (int i = l; i < r; ++i)
        v[i] += d;
    } else {
      int expected = std::accumulate(v.begin() + l, v.begin() + r, 0);
      int got = seg.range_query(l, r);
      EXPECT_EQ(got, expected)
          << "Mismatch on step " << step << " range [" << l << "," << r << ")";
    }
  }
}

TEST(container_lazy_segment_tree, identity_tag_is_noop) {
  std::vector<int> v = {5, 10, 15};
  auto apply = [](int v, size_t len, int tag) { return v + tag * (int)len; };
  LazySegmentTree<int, int, std::plus<int>, std::plus<int>, decltype(apply)>
      seg(v.begin(), v.end(), {}, {}, apply);

  seg.range_update(0, 3, 0); // no-op
  EXPECT_EQ(seg.range_query(0, 3), 30);
  seg.range_update(1, 1, 100); // empty range
  EXPECT_EQ(seg.range_query(0, 3), 30);
}

TEST(container_lazy_segment_tree, move_construction_and_assignment) {
  std::vector<int> v = {1, 2, 3};
  auto apply = [](int v, size_t len, int tag) { return v + tag * (int)len; };

  LazySegmentTree<int, int, std::plus<int>, std::plus<int>, decltype(apply)> a(
      v.begin(), v.end(), {}, {}, apply);

  a.range_update(0, 3, 1);
  int sum_before = a.range_query(0, 3);

  LazySegmentTree<int, int, std::plus<int>, std::plus<int>, decltype(apply)> b =
      std::move(a);

  EXPECT_EQ(b.range_query(0, 3), sum_before);
}

TEST(container_lazy_segment_tree, BasicPointSetAndQuery) {
  LazySegmentTree<int> seg(8);

  // all zeros initially
  EXPECT_EQ(seg.range_query(0, 8), 0);

  // point set
  seg.set(3, 5);
  seg.set(6, 10);

  EXPECT_EQ(seg.range_query(0, 8), 15);
  EXPECT_EQ(seg.range_query(3, 4), 5);
  EXPECT_EQ(seg.range_query(6, 7), 10);

  // overwrite
  seg.set(3, 20);
  EXPECT_EQ(seg.range_query(3, 4), 20);
  EXPECT_EQ(seg.range_query(0, 8), 30);
}

TEST(container_lazy_segment_tree, RangeUpdateSum) {
  LazySegmentTree<int> seg(10);

  // add +1 to range [0, 10)
  seg.range_update(0, 10, 1);
  EXPECT_EQ(seg.range_query(0, 10), 10);

  // add +2 to [2, 5)
  seg.range_update(2, 5, 2);
  // Expected total: 10 + 3*2 = 16
  EXPECT_EQ(seg.range_query(0, 10), 16);
  EXPECT_EQ(seg.range_query(2, 5), 3 * 3); // 1+2=3 each element
}

TEST(container_lazy_segment_tree, MixedLazyAndPoint) {
  LazySegmentTree<int> seg(6);

  seg.range_update(0, 6, 2); // +2 everywhere
  EXPECT_EQ(seg.range_query(0, 6), 12);

  // Overwrite index 3 to 100
  seg.set(3, 100);
  EXPECT_EQ(seg.range_query(3, 4), 100);

  // Ensure sums correct elsewhere
  EXPECT_EQ(seg.range_query(0, 3), 6);
  EXPECT_EQ(seg.range_query(0, 6), 110);
}

TEST(container_lazy_segment_tree, PropagationAndOverwrite) {
  LazySegmentTree<int> seg(4);

  seg.range_update(0, 4, 5);
  // Push lazy by querying partial
  EXPECT_EQ(seg.range_query(0, 2), 10);

  seg.set(1, 42);                 // overwrite single element
  EXPECT_EQ(seg.range_query(0, 2), 47); // 5 + 42
  EXPECT_EQ(seg.range_query(1, 2), 42);
}

struct MinValue {
  int value;
  MinValue(int v = std::numeric_limits<int>::max()) : value(v) {}
  friend bool operator==(const MinValue &a, const MinValue &b) {
    return a.value == b.value;
  }
};

TEST(container_lazy_segment_tree, MinOperator) {

  struct Combine {
    MinValue operator()(MinValue a, MinValue b) const {
      return MinValue(std::min(a.value, b.value));
    }
  };

  struct Apply {
    void operator()(MinValue& v, size_t, const MinValue &t) const {
      v =  MinValue(std::min(v.value, t.value));
    }
  };
  LazySegmentTree<MinValue, MinValue, Combine, Combine, Apply> seg(4);

  seg.set(0, 9);
  seg.set(1, 7);
  seg.set(2, 3);
  seg.set(3, 5);
  EXPECT_EQ(seg.range_query(0, 4), 3);

  seg.range_update(0, 4, MinValue(2));
  EXPECT_EQ(seg.range_query(0, 4), 2);
}

TEST(container_lazy_segment_tree, MultiSetWaveExample) {
  struct Chair {
    uint64_t position;
    uint64_t index;
  };
  struct ChairComp {
    bool operator()(const Chair &a, const Chair &b) const {
      return a.position == b.position ? a.index < b.index
                                      : a.position < b.position;
    }
  };
  using set = std::multiset<Chair, ChairComp>;

  struct Combine {
    set operator()(const set &a, const set &b) const {
      set res = a;
      res.merge(set(b));
      return res;
    }
  };
  struct Compose {
    uint64_t operator()(uint64_t a, uint64_t b) const { return std::max(a, b); }
  };
  struct Apply {
    void operator()(set &s, size_t, uint64_t tag) const {
      Chair key{tag, UINT64_MAX};
      s.erase(s.begin(), s.lower_bound(key));
    }
  };

  std::vector<set> v = {set{{{3, 0}}}, set{{{10, 1}}}, set{{{5, 2}}}};
  LazySegmentTree<set, uint64_t, Combine, Compose, Apply> seg(v.begin(),
                                                              v.end());

  // before wave
  EXPECT_EQ(seg.range_query(0, 3).size(), 3u);

  // apply wave strength 6 on all
  seg.range_update(0, 3, 6);

  // all chairs with position <=6 removed -> only {10,1} remains
  auto remaining = seg.range_query(0, 3);
  EXPECT_EQ(remaining.size(), 1u);
  EXPECT_EQ(remaining.begin()->position, 10u);
}
