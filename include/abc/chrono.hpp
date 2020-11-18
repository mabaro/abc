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
