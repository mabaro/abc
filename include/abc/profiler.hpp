#pragma once

#include "abc/debug.hpp"
#include "abc/format.hpp"
#include "abc/string.hpp"
#include "abc/timer.hpp"

#include <algorithm>
#include <iostream>
#include <mutex>
#include <vector>

namespace abc {
namespace detail {
///////////////////////////////////////////////////////////////////////////////

struct profiler {
protected:
    struct ProfilingData;
    using sample_container_t          = std::vector<ProfilingData>;
    using sample_container_iterator_t = sample_container_t::iterator;

public:
    static profiler& GetInstance()
    {
        static profiler theInstance;
        return theInstance;
    }

    void initialize() { }

    void print_summary(const std::vector<abc::string>& tagFilter = std::vector<abc::string>()) const
    {
        std::cout << "-----------------------------------------------------------------" << std::endl;
        std::cout << "-- Profiling summary" << std::endl;
        std::cout << "-----------------------------------------------------------------" << std::endl;

        const auto timeUnitsFunc = [](const abc::chrono::duration& duration) {
            if (duration >= abc::chrono::seconds(1)) {
                return abc::format("{} s", std::chrono::duration_cast<abc::chrono::secondsf>(duration).count());
            } else if (duration >= abc::chrono::milliseconds(1)) {
                return abc::format("{} ms", std::chrono::duration_cast<abc::chrono::millisecondsf>(duration).count());
            } else {
                return abc::format("{} us", std::chrono::duration_cast<abc::chrono::microsecondsf>(duration).count());
            }
        };

        const auto processSample = [&timeUnitsFunc](const ProfilingData& data) {
            if (data.samples <= 0) {
                return;
            }

            const auto avgTime = data.accumDuration / data.samples;
            if (data.samples > 1) {
                if (data.mt_lockedTime > ProfilingData::duration(0)) {
                    const auto lockedTime = data.mt_lockedTime / data.samples;
                    const auto avgTimeMT  = avgTime - lockedTime;

                    std::cout << abc::format("{} : avg({})lckd({}) min/max({}/{})#[{}]", data.tag,
                        timeUnitsFunc(avgTimeMT), timeUnitsFunc(lockedTime), timeUnitsFunc(data.minDuration),
                        timeUnitsFunc(data.maxDuration), data.samples)
                              << std::endl;
                } else {
                    std::cout << abc::format("{} : avg({}) min/max({}/{})#[{}]", data.tag, timeUnitsFunc(avgTime),
                        timeUnitsFunc(data.minDuration), timeUnitsFunc(data.maxDuration), data.samples)
                              << std::endl;
                }
            } else {
                if (data.mt_lockedTime > ProfilingData::duration(0)) {
                    const auto& lockedTime = data.mt_lockedTime;
                    const auto  avgTimeMT  = avgTime - lockedTime;
                    std::cout << abc::format(
                        "{} : {} (locked: {})", data.tag, timeUnitsFunc(avgTimeMT), timeUnitsFunc(lockedTime))
                              << std::endl;
                } else {
                    std::cout << abc::format("{} : {}", data.tag, timeUnitsFunc(avgTime)) << std::endl;
                }
            }
        };
        if (tagFilter.empty()) {
            for (const auto& data : m_samples) {
                processSample(data);
            }
        } else {
            for (const auto& tag : tagFilter) {
                auto it = std::find_if(
                    m_samples.begin(), m_samples.end(), [&tag](const ProfilingData& data) { return data.tag == tag; });
                if (it != m_samples.end()) {
                    processSample(*it);
                }
            }
        }
        std::cout << "-----------------------------------------------------------------" << std::endl;
    }

    void declare(const abc::string& tag, bool reset = false)
    {
        auto it = std::find_if(
            m_samples.begin(), m_samples.end(), [&tag](const ProfilingData& data) { return data.tag == tag; });
        if (it == m_samples.end()) {
            m_samples.emplace_back(tag);
        } else {
            if (reset) {
                m_samples.erase(it);
            } else {
                ABC_LOG_DEBUG("Call to profiler::declare({}) failed because the tag is already registered", tag);
            }
        }
    }

    void tick(const abc::string& tag) { internal_tick(tag); }
    void tock(const abc::string& tag) { internal_tock(tag); }

    void tick_mt(const abc::string& tag)
    {
        abc::chrono::timer          timer;
        std::lock_guard<std::mutex> mutexLock(m_mutex);
        auto                        lockedDuration = timer.get_elapsed_time();

        auto it = internal_tick(tag);

        ABC_ASSERT(it != m_samples.end());
        if (it != m_samples.end()) {   // accumulate locked duration
            it->mt_lockedTime += lockedDuration;
        }
    }
    void tock_mt(const abc::string& tag)
    {
        abc::chrono::timer          timer;
        std::lock_guard<std::mutex> mutexLock(m_mutex);
        auto                        lockedDuration = timer.get_elapsed_time();

        auto it = internal_tock(tag);

        ABC_ASSERT(it != m_samples.end());
        if (it != m_samples.end()) {   // accumulate locked duration
            it->mt_lockedTime += lockedDuration;
        }
    }

protected:
    sample_container_iterator_t internal_tick(const abc::string& tag)
    {
        sample_container_iterator_t it = std::find_if(
            m_samples.begin(), m_samples.end(), [&tag](const ProfilingData& data) { return data.tag == tag; });
        if (it != m_samples.end()) {
            ABC_ASSERT(it->t0 == abc::chrono::timer::time_point_t(), "Call to PROFILE_BEGIN without PROFILE_END.");
            it->t0 = abc::chrono::timer::now();
        } else {
            m_samples.emplace_back(tag, abc::chrono::timer::now());
        }

        return it;
    }

    sample_container_iterator_t internal_tock(const abc::string& tag)
    {
        sample_container_iterator_t it = std::find_if(
            m_samples.begin(), m_samples.end(), [&tag](const ProfilingData& data) { return data.tag == tag; });
        if (it != m_samples.end()) {
            ABC_ASSERT(it->t0 != abc::chrono::timer::time_point_t(), "Call to PROFILE_END without PROFILE_BEGIN.");
            const abc::chrono::duration elapsedTime = abc::chrono::timer::now() - it->t0;
            it->minDuration                         = (it->minDuration < elapsedTime) ? it->minDuration : elapsedTime;
            it->maxDuration                         = (it->maxDuration > elapsedTime) ? it->maxDuration : elapsedTime;
            it->accumDuration += elapsedTime;
            it->t0 = abc::chrono::timer::time_point_t();
            ++(it->samples);
        } else {
            ABC_FAIL("Call to PROFILE_END without PROFILE_BEGIN.");
        }

        return it;
    }

protected:
    std::mutex m_mutex;

    struct ProfilingData {
        using duration   = abc::chrono::timer::duration_t;
        using time_point = abc::chrono::timer::time_point_t;

        ProfilingData(const abc::string& i_tag)
            : tag(i_tag)
        {
        }
        ProfilingData(const abc::string& i_tag, const time_point& i_t0)
            : tag(i_tag)
            , t0(i_t0)
        {
        }

        abc::string tag;
        time_point  t0;
        duration    accumDuration = duration(0);
        duration    minDuration   = duration(std::numeric_limits<duration::rep>::max());
        duration    maxDuration   = duration(0);
        size_t      samples       = 0;

        duration mt_lockedTime = duration(0);
    };
    sample_container_t m_samples;
};

///////////////////////////////////////////////////////////////////////////////
}   // namespace detail
}   // namespace abc

///////////////////////////////////////////////////////////////////////////////
#define ABC_PROFILE_INIT() abc::detail::profiler::GetInstance().initialize()
///////////////////////////////////////////////////////////////////////////////
#define ABC_PROFILE_SECTION(TAG, CODE_BLOCK)             \
    do {                                                 \
        abc::detail::profiler::GetInstance().tick(#TAG); \
        sizeof(#CODE_BLOCK);                             \
        {CODE_BLOCK};                                    \
        abc::detail::profiler::GetInstance().tock(#TAG); \
    } while (false)
///////////////////////////////////////////////////////////////////////////////
#define ABC_PROFILE_DECLARE(TAG)                            \
    do {                                                    \
        abc::detail::profiler::GetInstance().declare(#TAG); \
    } while (false)

#define ABC_PROFILE_BEGIN(TAG)                           \
    do {                                                 \
        abc::detail::profiler::GetInstance().tick(#TAG); \
    } while (false)
#define ABC_PROFILE_END(TAG)                             \
    do {                                                 \
        abc::detail::profiler::GetInstance().tock(#TAG); \
    } while (false)

#define ABC_PROFILE_BEGIN_MT(TAG)                           \
    do {                                                    \
        abc::detail::profiler::GetInstance().tick_mt(#TAG); \
    } while (false)
#define PROFILE_END_MT(TAG)                                 \
    do {                                                    \
        abc::detail::profiler::GetInstance().tock_mt(#TAG); \
    } while (false)

#define ABC_PROFILE_SUMMARY(...)                                           \
    do {                                                                   \
        abc::detail::profiler::GetInstance().print_summary(##__VA_ARGS__); \
    } while (false)
