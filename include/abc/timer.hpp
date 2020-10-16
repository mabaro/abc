#pragma once

#include "abc/chrono.hpp"
#include <chrono>

namespace abc {
namespace time {
///////////////////////////////////////////////////////////////////////////////

namespace detail {
///////////////////////////////////////////////////////////////////////////////

template <typename ClockT = typename chrono::clock>
struct TimerBase {
  using clock_t      = ClockT;
  using duration_t   = typename clock_t::duration_t;
  using time_point_t = typename clock_t::time_point_t;

  time_point_t m_start;

  static time_point_t Now() { return clock_t::now(); }

  TimerBase() : m_start(clock_t::now()) {}

  void Reset() { m_start = clock_t::now(); }

  duration_t GetElapsedTime() const { return clock_t::now() - m_start; }

  template <typename DurationT>
  DurationT GetElapsedTimeAs() const {
    return std::chrono::duration_cast<DurationT>(GetElapsedTime());
  }
};

///////////////////////////////////////////////////////////////////////////////
}  // namespace detail

using Timer = detail::TimerBase<>;

using duration     = Timer::duration_t;
using time_point   = Timer::time_point_t;
using nanoseconds  = std::chrono::nanoseconds;
using microseconds = std::chrono::microseconds;
using milliseconds = std::chrono::milliseconds;
using seconds      = std::chrono::seconds;
using minutes      = std::chrono::minutes;
using hours        = std::chrono::hours;

///////////////////////////////////////////////////////////////////////////////
}  // namespace time
}  // namespace abc
