#pragma once

#include "abc/chrono.hpp"
#include <chrono>

namespace abc {
namespace chrono {
///////////////////////////////////////////////////////////////////////////////

namespace detail {
///////////////////////////////////////////////////////////////////////////////

template <typename ClockT = typename chrono::clock>
struct timer_base {
  using clock_t      = ClockT;
  using duration_t   = typename clock_t::duration_t;
  using time_point_t = typename clock_t::time_point_t;

  time_point_t m_start;

  static time_point_t now() { return clock_t::now(); }

  timer_base() : m_start(clock_t::now()) {}

  void reset() { m_start = clock_t::now(); }

  duration_t get_elapsed_time() const { return clock_t::now() - m_start; }

  template <typename DurationT>
  DurationT get_elapsed_time_as() const {
    return std::chrono::duration_cast<DurationT>(get_elapsed_time());
  }
};

///////////////////////////////////////////////////////////////////////////////
}  // namespace detail

using timer = detail::timer_base<>;

using duration      = timer::duration_t;
using time_point    = timer::time_point_t;

using nanoseconds   = std::chrono::nanoseconds;
using microseconds  = std::chrono::microseconds;
using milliseconds  = std::chrono::milliseconds;
using seconds       = std::chrono::seconds;
using minutes       = std::chrono::minutes;
using hours         = std::chrono::hours;

using microsecondsf = std::chrono::duration<float, std::micro>;
using millisecondsf = std::chrono::duration<float, std::milli>;
using secondsf      = std::chrono::duration<float>;
using minutesf      = std::chrono::duration<float, std::ratio<60, 1>>;
using hoursf        = std::chrono::duration<float, std::ratio<3600, 1>>;

///////////////////////////////////////////////////////////////////////////////
}  // namespace time
}  // namespace abc
