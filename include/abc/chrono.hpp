#pragma once

#include <chrono>

namespace abc
{
namespace chrono
{
///////////////////////////////////////////////////////////////////////////////

template<typename ClockT,
         typename TRep = typename ClockT::rep,
         typename TPeriod = typename ClockT::period,
         typename TDuration = typename ClockT::duration,
         typename TTimepoint = typename ClockT::time_point>
class ClockBase : protected ClockT
{
public:
	using clock_t = ClockT;
	using rep_t = TRep;
	using period_t = TPeriod;
	using duration_t = TDuration;
	using time_point_t = TTimepoint;

	virtual ~ClockBase() = default;

	using clock_t::now;
};

using clock = ClockBase<std::chrono::high_resolution_clock>;
using duration = clock::duration_t;
using time_point = clock::time_point_t;

using nanoseconds  = std::chrono::nanoseconds;
using microseconds = std::chrono::microseconds;
using milliseconds = std::chrono::milliseconds;
using seconds      = std::chrono::seconds;
using minutes = std::chrono::minutes;
using hours = std::chrono::hours;

using nanosecondsf  = std::chrono::duration<float, std::nano>;
using microsecondsf = std::chrono::duration<float, std::micro>;
using millisecondsf = std::chrono::duration<float, std::milli>;
using secondsf      = std::chrono::duration<float>;
using minutesf      = std::chrono::duration<float, std::ratio<60>>;
using hoursf        = std::chrono::duration<float, std::ratio<3600>>;

///////////////////////////////////////////////////////////////////////////////
}//chrono
}//abc

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

namespace abc{
namespace detail{
///////////////////////////////////////////////////////////////////////////////

template <class R, class P>
struct to_string_impl<std::chrono::duration<R, P>> {
  static abc::string impl(const std::chrono::duration<R, P>& duration) {
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration);

    size_t seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
    millis -= abc::chrono::seconds(seconds);

    const size_t daySeconds = (3600 * 24);
    size_t       days       = seconds / daySeconds;
    seconds -= days * daySeconds;

    const size_t hourSeconds = 3600;
    size_t       hours       = seconds / hourSeconds;
    seconds -= hours * hourSeconds;

    const size_t minuteSeconds = 60;
    size_t       minutes       = seconds / minuteSeconds;
    seconds -= minutes * minuteSeconds;

    abc::string result;
    const bool  hasDays    = days > 0;
    const bool  hasHours   = hours > 0;
    const bool  hasMinutes = minutes > 0;
    const bool  hasSeconds = seconds > 0;
    const bool  hasMillis  = millis.count() > 0;

    const bool  showDays    = hasDays;
    const bool  showHours   = hasHours   || hasDays && hasMinutes;
    const bool  showMinutes = hasMinutes || (hasDays || hasHours) && hasSeconds;
    const bool  showSeconds = hasSeconds || (hasDays || hasHours || hasMinutes) && hasMillis;
    const bool  showMillis  = hasMillis;

    bool hasPrior = false;
    if (showDays) {
      result += abc::format("{}", days);
      result += "d";
      hasPrior = true;
    }
    if (showHours) {
      result += abc::format("{}{}", (hasPrior ? " " : ""), hours);
      if (!hasPrior) { result += "h"; }
      hasPrior = true;
    }
    if (showMinutes) {
      result += abc::format("{}{}", (hasPrior ? ":" : ""), minutes);
      if (!hasPrior) { result += "min"; }
      hasPrior  = true;
    }
    if (showSeconds) {
      result += abc::format("{}{}", (hasPrior ? ":" : ""), seconds);
      if (!hasPrior) { result += "s"; }
      hasPrior  = true;
    }
    if (showMillis) {
      result += abc::format("{}{}", (hasPrior ? "." : ""), millis.count());
      if (!hasPrior) { result += "ms"; }
    }
    if (!hasPrior) { result += "0s"; }

    return result;
  }
};

///////////////////////////////////////////////////////////////////////////////
}  // namespace detail
}  // namespace abc
