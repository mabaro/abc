# set of options that may affect compilation flags

option(ENABLE_EXCEPTIONS "Enable exceptions" OFF)
option(ENABLE_RTTI "Enable RTTI" OFF)
option(ENABLE_PCH "Enable Precompiled Headers" ON)

# ##############################################################################
# COMMON LANGUAGE FEATURES
# https://cmake.org/cmake/help/v3.8/prop_gbl/CMAKE_CXX_KNOWN_FEATURES.html#prop_gbl:CMAKE_CXX_KNOWN_FEATURES
# language features is too fine grain - on a big codebase is impossible to
# manage correctly also you dont want parts of your codebase to be compiled with
# different versions of the cppstd - We require C++17 and the reason is the
# templated constants struct which contains static constexpr members Until C++17
# (so C++11/C++14) it was mandatory to instantiate the static member outside of
# their class. So we should instantiate all members of each type T on the public
# namespace!!! This is not feasible on a templated function.
#
# So a solution would be: a) ditch the `static constexpr` members and move them
# to `constexpr` functions to try to hide instantiations. b) Use C++17 where a
# change on the standard was introduced to make `static constexpr` members also
# implicitly `inline` and they get auto-defined.
# http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0386r2.pdf

target_compile_features(project_options INTERFACE cxx_std_17)
target_compile_options(project_options INTERFACE)

# ##############################################################################

# handle rrti using targets
if(ENABLE_RTTI)

  set(MSVC_RTTI_ON_FLAG /GR)
  set(GCC_RTTI_ON_FLAG -frtti)

  target_compile_options(
    project_options
    INTERFACE $<$<CXX_COMPILER_ID:MSVC>:${MSVC_RTTI_ON_FLAG}>
              $<$<CXX_COMPILER_ID:Clang>:${GCC_RTTI_ON_FLAG}>
              $<$<CXX_COMPILER_ID:AppleClang>:${GCC_RTTI_ON_FLAG}>
              $<$<CXX_COMPILER_ID:GNU>:${GCC_RTTI_ON_FLAG}>)
  add_compile_options(
    $<$<CXX_COMPILER_ID:MSVC>:${MSVC_RTTI_ON_FLAG}>
    $<$<CXX_COMPILER_ID:Clang>:${GCC_RTTI_ON_FLAG}>
    $<$<CXX_COMPILER_ID:AppleClang>:${GCC_RTTI_ON_FLAG}>
    $<$<CXX_COMPILER_ID:GNU>:${GCC_RTTI_ON_FLAG}>)
else()
  message(STATUS "* RTTI is disabled")

  set(MSVC_RTTI_OFF_FLAG /GR-)
  set(GCC_RTTI_OFF_FLAG -fno-rtti)

  if(MSVC)
    string(REGEX REPLACE "/GR" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  endif()

  target_compile_options(
    project_options
    INTERFACE $<$<CXX_COMPILER_ID:MSVC>:${MSVC_RTTI_OFF_FLAG}>
              $<$<CXX_COMPILER_ID:Clang>:${GCC_RTTI_OFF_FLAG}>
              $<$<CXX_COMPILER_ID:AppleClang>:${GCC_RTTI_OFF_FLAG}>
              $<$<CXX_COMPILER_ID:GNU>:${GCC_RTTI_OFF_FLAG}>)
  # This is global -> remove and link with project_options
  add_compile_options(
    $<$<CXX_COMPILER_ID:MSVC>:${MSVC_RTTI_OFF_FLAG}>
    $<$<CXX_COMPILER_ID:Clang>:${GCC_RTTI_OFF_FLAG}>
    $<$<CXX_COMPILER_ID:AppleClang>:${GCC_RTTI_OFF_FLAG}>
    $<$<CXX_COMPILER_ID:GNU>:${GCC_RTTI_OFF_FLAG}>)
endif()

# handle exceptions flag using targets
if(NOT ENABLE_EXCEPTIONS)
  message(
    STATUS "Exceptions have been disabled. Any operation that would "
           "throw an exception will result in a call to std::abort() instead.")

  if(MSVC)
    string(REGEX REPLACE "/EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  endif()

  set(MSVC_EXCEPTIONS_FLAG /EHs-c-)
  set(MSVC_EXCEPTIONS_DEFINITIONS _HAS_EXCEPTIONS=0)

  set(GCC_EXCEPTIONS_FLAG -fno-exceptions -fno-unwind-tables
                          -fno-asynchronous-unwind-tables)

  # This is global -> remove and link with project_options
  add_compile_definitions(
    # $<$<CXX_COMPILER_ID:MSVC>:${MSVC_EXCEPTIONS_DEFINITIONS}>
  )
  add_compile_options(
    # $<$<CXX_COMPILER_ID:MSVC>:${MSVC_EXCEPTIONS_FLAG}>
    # $<$<CXX_COMPILER_ID:Clang>:${GCC_EXCEPTIONS_FLAG}>
    # $<$<CXX_COMPILER_ID:AppleClang>:${GCC_EXCEPTIONS_FLAG}>
    # $<$<CXX_COMPILER_ID:GNU>:${GCC_EXCEPTIONS_FLAG}>
  )

  target_compile_definitions(
    project_options
    INTERFACE $<$<CXX_COMPILER_ID:MSVC>:${MSVC_EXCEPTIONS_DEFINITIONS}>)
  target_compile_options(
    project_options
    INTERFACE $<$<CXX_COMPILER_ID:MSVC>:${MSVC_EXCEPTIONS_FLAG}>
              $<$<CXX_COMPILER_ID:Clang>:${GCC_EXCEPTIONS_FLAG}>
              $<$<CXX_COMPILER_ID:AppleClang>:${GCC_EXCEPTIONS_FLAG}>
              $<$<CXX_COMPILER_ID:GNU>:${GCC_EXCEPTIONS_FLAG}>)
endif()

# Very basic PCH example
if(ENABLE_PCH)
  # This sets a global PCH parameter, each project will build its own PCH, which
  # is a good idea if any #define's change
  #
  target_precompile_headers(
    project_options
      INTERFACE
        <vector>
        <string>
        <map>
        <utility>
        "include/abc/core.hpp"
        "include/abc/debug.hpp"
        "include/abc/pointer.hpp"
        "include/abc/string.hpp"
        "include/abc/format.hpp"
  )
endif()
