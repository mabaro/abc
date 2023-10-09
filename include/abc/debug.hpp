// The helpers defined in this file are used all around the codebase,
// thus it must be the first dependency header to include
// At the same time, the helpers in this file do depend on string/format,
// therefore, these dependencies will be kept at the end of this file
#pragma once

#include "abc/core.hpp"

#ifndef ABC_CORE_INCLUDED
#error "abc/core.hpp" needs to be included before this file.
#endif

#include <iostream>

#if defined(ABC_FORCE_DEBUG) && defined(ABC_RETAIL)
#error "ABC_FORCE_DEBUG and ABC_RETAIL are incompatible!"
#endif

#if defined(ABC_DEBUG)
#error "Do not pass ABC_DEBUG to the compiler. If you want to force debug, pass ABC_FORCE_DEBUG"
#endif

#ifndef ABC_RELEASE_CHECKED
#if !defined(NDEBUG) || defined(ABC_FORCE_DEBUG)
#define ABC_DEBUG
#else
#define ABC_RELEASE
#endif
#endif

#if defined(ABC_DEBUG) || defined(ABC_RELEASE_CHECKED)
#if defined(ABC_PLATFORM_WINDOWS_FAMILY)
#define ABC_BREAK() \
    __debugbreak()   // better than assert(0) as it will break on this exact line instead of deep
                     // in the assert function.
#elif defined(ABC_PLATFORM_OSX_FAMILY) || defined(ABC_PLATFORM_LINUX_FAMILY)
#include <signal.h>
#define ABC_BREAK() raise(SIGTRAP)
#elif defined(ABC_PLATFORM_ANDROID_FAMILY)
#include <signal.h>
#define ABC_BREAK() raise(SIGSTOP)
#else
#error "Unsupported platform"
#endif
#else
#define ABC_BREAK()
#endif

namespace abc {
namespace assert {
namespace detail {
enum class response {
    BREAK,
    CONTINUE
};
response do_assert(const char* condition, const char* file, int line, const char* message);

template <typename PARAM> struct is_param_string {
    static constexpr bool value = false;
};
template <> struct is_param_string<char> {
    static constexpr bool value = true;
};
template <size_t N> struct is_param_string<char[N]> {
    static constexpr bool value = true;
};

}   // namespace detail
}   // namespace assert
}   // namespace abc

/// Log
#define ABC_LOG_ERROR_IMPL(MSG, ...)                                                                                   \
    do {                                                                                                               \
        std::cerr << abc::format("[ERROR][{}:{}] {}", __FILE__, __LINE__, abc::format(MSG, __VA_ARGS__)) << std::endl; \
    } while (0)

#define ABC_LOG_WARNING_IMPL(MSG, ...)                                                                     \
    do {                                                                                                   \
        std::cerr << abc::format("[WARNING][{}:{}] {}", __FILE__, __LINE__, abc::format(MSG, __VA_ARGS__)) \
                  << std::endl;                                                                            \
    } while (0)

#define ABC_LOG_DEBUG_IMPL(MSG, ...)                                                                                   \
    do {                                                                                                               \
        std::cerr << abc::format("[DEBUG][{}:{}] {}", __FILE__, __LINE__, abc::format(MSG, __VA_ARGS__)) << std::endl; \
    } while (0)

#define ABC_LOG_INFO_IMPL(MSG, ...)                                                                                   \
    do {                                                                                                              \
        std::cerr << abc::format("[INFO][{}:{}] {}", __FILE__, __LINE__, abc::format(MSG, __VA_ARGS__)) << std::endl; \
    } while (0)

/// Assertions
#define ABC_ASSERT_IMPL(condition, ...)                                                        \
    do {                                                                                       \
        const auto& __cond_result__ = (condition);                                             \
        static_assert(!abc::assert::detail::is_param_string<decltype(__cond_result__)>::value, \
            "Condition is a string, probably swapped parameter order");                        \
        if (!(condition)) {                                                                    \
            abc::string msg = abc::format<abc::string>(__VA_ARGS__);                           \
            if (abc::assert::detail::do_assert(#condition, __FILE__, __LINE__, msg.data())     \
                == abc::assert::detail::response::BREAK) {                                     \
                ABC_BREAK();                                                                   \
            }                                                                                  \
        }                                                                                      \
    } while (false)

#define ABC_FAIL_IMPL(...)                                                                      \
    do {                                                                                        \
        abc::string msg = abc::format<abc::string>(__VA_ARGS__);                                \
        std::cerr << "FAIL: " << msg << "[" << __FILE__ << ":" << __LINE__ << "]" << std::endl; \
        ABC_BREAK();                                                                            \
    } while (false)

///////////////////////////////////////////////////////////////////////////////

#if defined(ABC_DEBUG) || defined(ABC_RELEASE_CHECKED)
#define ABC_ASSERT(_condition_, ...)         ABC_ASSERT_IMPL(_condition_, ##__VA_ARGS__)
#define ABC_FAIL                             ABC_FAIL_IMPL
#define ABC_ASSERT_RELEASE(_condition_, ...) ABC_ASSERT_IMPL(_condition_, ##__VA_ARGS__)
#define ABC_FAIL_RELEASE                     ABC_FAIL_IMPL

#define ABC_LOG_ERROR(MSG, ...)   ABC_LOG_ERROR_IMPL(MSG, ##__VA_ARGS__)
#define ABC_LOG_WARNING(MSG, ...) ABC_LOG_WARNING_IMPL(MSG, ##__VA_ARGS__)
#define ABC_LOG_INFO(MSG, ...)    ABC_LOG_INFO_IMPL(MSG, ##__VA_ARGS__)
#define ABC_LOG_DEBUG(MSG, ...)   ABC_LOG_DEBUG_IMPL(MSG, ##__VA_ARGS__)
#else
#define ABC_ASSERT(...)
#define ABC_FAIL(...)
#define ABC_ASSERT_RELEASE(_condition_, ...) ABC_ASSERT_IMPL(_condition_, ##__VA_ARGS__)
#define ABC_FAIL_RELEASE                     ABC_FAIL_IMPL

#define ABC_LOG_ERROR(MSG, ...)   ABC_LOG_ERROR_IMPL(MSG, ##__VA_ARGS__)
#define ABC_LOG_WARNING(MSG, ...) ABC_LOG_WARNING_IMPL(MSG, ##__VA_ARGS__)
#define ABC_LOG_INFO(MSG, ...)    ABC_LOG_INFO_IMPL(MSG, ##__VA_ARGS__)
#define ABC_LOG_DEBUG(...)
#endif

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#include "abc/format.hpp"
#include "abc/string.hpp"
