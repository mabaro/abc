#pragma once

#include "abc/platform/platform.hpp"
#include <functional>

#define ABC_CORE_INCLUDED

#ifdef ABC_UNUSED_DECL
#elif (__GNUC__)
#if defined(__clang__)
#define ABC_UNUSED_DECL(x) x __attribute__((unused))
#else
#define ABC_UNUSED_DECL(__X__) __X__ __attribute__((unused))
#endif   // #if (__GNUC__)
#else
#define ABC_UNUSED_DECL(x) x
#endif   // #ifndef ABC_UNUSED_DECL

#ifndef ABC_UNUSED
#define ABC_UNUSED(x) (void)(x)
#endif   // #ifndef ABC_UNUSED

#define ABC_NOTHROW   noexcept
#define ABC_NODISCARD [[nodiscard]]

// https://foonathan.net/2020/09/move-forward/
#define ABC_MOVE(...)    static_cast<std::remove_reference_t<decltype(__VA_ARGS__)>&&>(__VA_ARGS__)
#define ABC_FORWARD(...) static_cast<decltype(__VA_ARGS__)&&>(__VA_ARGS__)

namespace abc {
//////////////////////////////////////////////////////////////////////////

struct success_t { };
static success_t ABC_UNUSED_DECL(success);

struct uninitialized_t { };
static uninitialized_t ABC_UNUSED_DECL(uninitialized);

template <class T> using function = std::function<T>;

//////////////////////////////////////////////////////////////////////////

namespace detail {
//////////////////////////////////////////////////////////////////////////

class noncopyable {
public:
    noncopyable(const noncopyable&)            = delete;
    noncopyable& operator=(const noncopyable&) = delete;

protected:
    noncopyable()  = default;
    ~noncopyable() = default;
};

//////////////////////////////////////////////////////////////////////////
}   // namespace detail

//////////////////////////////////////////////////////////////////////////

namespace helpers {
//////////////////////////////////////////////////////////////////////////

template <typename T> struct fail {
    static const bool value = std::is_same<T, T>::value == false;
};

template <typename T1, typename T2, bool SELECTOR = true> struct select_type {
    using type = T1;
};
template <typename T1, typename T2> struct select_type<T1, T2, false> {
    using type = T2;
};

//////////////////////////////////////////////////////////////////////////
}   // namespace helpers

using noncopyable = detail::noncopyable;

//////////////////////////////////////////////////////////////////////////
}   // namespace abc
