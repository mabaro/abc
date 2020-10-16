#include "doctest/doctest.h"

#include "abc/core.hpp"
#include "abc/utils.hpp"

template <typename T, typename... Args>
T dummy_variadic(const Args &... args)
{
  return abc::detail::get_variadic_argument<T>(3, args...);
}

TEST_CASE("abc - get_variadic_argument")
{
  using namespace abc;

  CHECK(dummy_variadic<int>(1, 2, 3, 4, 5, 6, 7) == 4);
}
