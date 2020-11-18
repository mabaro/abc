#pragma once

#include "abc/optional.hpp"
#include "abc/tagged_type.hpp"
#include "abc/format_chrono.hpp"

//#include <cmath>

namespace abc
{
namespace detail
{
//////////////////////////////////////////////////////////////////////////

template <>
struct to_string_impl<int64_t> {
  static abc::string impl(const int64_t v) {
    if (v == 0) { return "0"; }

    abc::string  result;
    int64_t      temp = v;
    char         str[16];
    const int16_t numDigits = 1 + static_cast<int16_t>(v > 0 ? std::log10(v) : 1 + std::log10(-v));
    ABC_ASSERT(numDigits <= 16);

    int16_t index = numDigits - 1;

    int64_t localV = v;
    if (v < 0) {
      localV            = -v;
      str[index++]      = '-';
    }

    do {
      str[index] = '0' + (temp % 10);
      index      = 0x0F & (index - 1);
      temp /= 10;
    } while (temp > 0);

    return abc::string(&str[0], numDigits);
  }
};

template <typename T, typename TTag, int64_t DefaultValue>
struct to_string_impl<tagged_type<T, TTag, DefaultValue>> {
  using target_t = typename abc::helpers::select_type<int64_t, uint64_t, std::is_signed<T>::value>::type;
  static abc::string impl(const tagged_type<T, TTag, DefaultValue>& v) {
    return to_string<target_t>(v.value());
  }
};

template <typename T>
struct to_string_impl<optional<T>> {
  static abc::string impl(const optional<T>& v) {
    return optional.has_value()
	? to_string(v.value())
	: "OPT_EMPTY";
  }
};

template <typename TErrorCode, typename... TInnerErrors>
struct to_string_impl<error<TErrorCode, TInnerErrors...>>
{
    using error_t = error<TErrorCode, TInnerErrors...>;
    static abc::string impl(const error_t& e)
    {
        return e.message_with_inner();
    }
};

//////////////////////////////////////////////////////////////////////////
}  // namespace detail
} // abc
 