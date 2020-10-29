#pragma once
#define ABC_CORE_INCLUDED

#include "abc/platform.hpp"
#include <functional>

#define ABC_UNUSED(X) (void)(X);

// https://foonathan.net/2020/09/move-forward/
#define ABC_MOVE(...) \ 
  static_cast<std::remove_reference_t<decltype(__VA_ARGS__)>&&>(__VA_ARGS__)
#define ABC_FORWARD(...) \
  static_cast<decltype(__VA_ARGS__)&&>(__VA_ARGS__)

namespace abc
{
//////////////////////////////////////////////////////////////////////////

static struct success_t {} success;
static struct uninitialized_t {} uninitialized;

template <class T>
using function = std::function<T>;

//////////////////////////////////////////////////////////////////////////

namespace detail
{
//////////////////////////////////////////////////////////////////////////

class noncopyable
{
public:
    noncopyable(const noncopyable &) = delete;
    noncopyable &operator=(const noncopyable &) = delete;

protected:
    noncopyable()  = default;
    ~noncopyable() = default;
};

//////////////////////////////////////////////////////////////////////////
}  // namespace detail

//////////////////////////////////////////////////////////////////////////

namespace helpers
{
//////////////////////////////////////////////////////////////////////////

template <typename T>
struct fail
{
    static const bool value = std::is_same<T, T>::value == false;
};

template <typename T1, typename T2, bool SELECTOR = true>
struct select_type
{
    using type = T1;
};
template <typename T1, typename T2>
struct select_type<T1, T2, false>
{
    using type = T2;
};

//////////////////////////////////////////////////////////////////////////
}  // namespace helpers

using noncopyable = detail::noncopyable;

//////////////////////////////////////////////////////////////////////////
}  // namespace abc

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "abc/platform.hpp"
#include "abc/debug.hpp"
#include "abc/pointer.hpp"
