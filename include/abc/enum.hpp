#pragma once

#include "abc/core.hpp"
#include "abc/string.hpp"
#include "abc/format.hpp"
#include "abc/utils.hpp"  //algo::split

#include <array>

namespace abc {
namespace detail {
///////////////////////////////////////////////////////////////////////////////
void build_enum_strings(const char* str, abc::string o_strings[], size_t i_expectedStringCount);
///////////////////////////////////////////////////////////////////////////////
}  // namespace detail
}  // namespace abc

///////////////////////////////////////////////////////////////////////////////

#define ABC_ENUM(ENUM_NAME, ...)                                                     \
  enum class ENUM_NAME { __VA_ARGS__, COUNT, FIRST = 0, LAST = COUNT - 1 };          \
  static const std::array<abc::string, static_cast<size_t>(ENUM_NAME::COUNT)>&       \
      __GET_##ENUM_NAME##_names() {                                                  \
    static std::array<abc::string, static_cast<size_t>(ENUM_NAME::COUNT)> strings;   \
    if (strings.front().empty()) {                                                   \
      abc::detail::build_enum_strings(#__VA_ARGS__, strings.data(), strings.size()); \
    }                                                                                \
    return strings;                                                                  \
  }

#define ABC_ENUM_WITH_TYPE(ENUM_NAME, ENUM_TYPE, ...)                                 \
  enum class #ENUM_NAME : ENUM_TYPE{__VA_ARGS__, COUNT, FIRST = 0, LAST = COUNT - 1}; \
  static const std::array<abc::string, static_cast<size_t>(ENUM_NAME::COUNT)>&        \
      __GET_##ENUM_NAME##_names() {                                                   \
    static std::array<abc::string, static_cast<size_t>(ENUM_NAME::COUNT)> strings;    \
    if (strings.front().empty()) {                                                    \
      abc::detail::build_enum_strings(#__VA_ARGS__, strings.data(), strings.size());  \
    }                                                                                 \
    return strings;                                                                   \
  }

#define ABC_ENUM_STRING_IMPL(SCOPE, ENUM_NAME)                                \
  namespace abc {                                                             \
  namespace detail {                                                          \
  static string to_string_impl(const SCOPE::ENUM_NAME& enumType) {            \
    if (enumType == SCOPE::ENUM_NAME::COUNT) { return "COUNT"; }              \
    return SCOPE::__GET_##ENUM_NAME##_names()[static_cast<size_t>(enumType)]; \
  }                                                                           \
  }                                                                           \
  template <>                                                                 \
  string to_string<SCOPE::ENUM_NAME>(const SCOPE::ENUM_NAME& enumType) {      \
    return detail::to_string_impl(enumType);                                  \
  }                                                                           \
  }
