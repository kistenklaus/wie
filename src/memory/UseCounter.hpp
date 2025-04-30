#pragma once

#include <atomic>
#include <concepts>
namespace strobe {

template <std::unsigned_integral T>
class UseCounter {
 public:
  UseCounter() : m_counter(1) {}
  void reset() { m_counter.store(1); }
  bool inc() {
    auto current = m_counter.load();
    while (current > 0 &&
           !m_counter.compare_exchange_weak(current, current + 1));
    return current > 0;
  }

  bool dec() { return m_counter.fetch_sub(1) == 1; }

  T useCount() const { return m_counter.load(); }

 private:
  std::atomic<T> m_counter;
};

}  // namespace strobe

