#pragma once

#include "abc/platform/platform.hpp"
#include <functional>

#define ABC_CORE_INCLUDED

#define ABC_UNUSED(X) (void)(X);
#define ABC_NOTHROW   noexcept
#define ABC_NODISCARD [[nodiscard]]

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
