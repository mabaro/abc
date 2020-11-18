#pragma once

#include "abc/chrono.hpp"
namespace abc{
namespace detail{
///////////////////////////////////////////////////////////////////////////////

template <class R, class P>
struct to_string_impl<std::chrono::duration<R, P>, false> {
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
