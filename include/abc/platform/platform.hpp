#pragma once

////////////////////////////////////////////////////////////////////////////////
// C++ 
////////////////////////////////////////////////////////////////////////////////
#if __cplusplus == 199711L
#  define ABC_CPP98
#elif __cplusplus == 201103L
#  define ABC_CPP11
#elif __cplusplus == 201402L
#  define ABC_CPP14
#elif __cplusplus == 201703L
#  define ABC_CPP17
#  define ABC_OPTIONAL_SUPPORTED
#endif

////////////////////////////////////////////////////////////////////////////////
// ARCHITECTURE
////////////////////////////////////////////////////////////////////////////////

#if defined(__arm64__) || defined(__aarch64__) || defined(_M_ARM64)
#  define ABC_PLATFORM_64
#  define ABC_PLATFORM_ARCHITECTURE_ARM64
#elif defined(__arm__) || defined(_M_ARM)
#  define ABC_PLATFORM_32
#  define ABC_PLATFORM_ARCHITECTURE_ARM32
#elif defined(__i386__) || defined(_M_IX86)
#  define ABC_PLATFORM_32
#  define ABC_PLATFORM_ARCHITECTURE_X86
#elif defined(__x86_64__) || defined(_M_AMD64)
#  define ABC_PLATFORM_64
#  define ABC_PLATFORM_ARCHITECTURE_AMD64
#else
#  pragma error "Unrecognized architecture "
#endif

//////////////////////////////////////////////////////////////////////////
// ANDROID
//////////////////////////////////////////////////////////////////////////

#if defined(__ANDROID__)

#  define ABC_PLATFORM_POSIX_THREADS_API
#  define ABC_PLATFORM_ANDROID_FAMILY
#  define ABC_PLATFORM_ANDROID_APP
#  define ABC_PLATFORM_POSIX_API

#elif defined(_WIN32) || defined(_WIN64) || defined(__MINGW64__)
#  define ABC_PLATFORM_WINDOWS_FAMILY
#  define ABC_PLATFORM_WIN_API
#  define ABC_PLATFORM_WINDOWS_THREADS_API
 
#  if defined(_MSC_VER)
#    pragma warning(disable : 4100)
#    pragma warning(disable : 4251)
#  endif
 
#  if defined(WINAPI_FAMILY)
#    include <winapifamily.h>
#    if (WINAPI_FAMILY == WINAPI_FAMILY_PC_APP)
//////////////////////////////////////////////////////////////////////////
// WINDOWS STORE
//////////////////////////////////////////////////////////////////////////

#      define ABC_PLATFORM_WINDOWS_STORE_APP
#    elif (WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP)
//////////////////////////////////////////////////////////////////////////
// WINDOWS PHONE
//////////////////////////////////////////////////////////////////////////s

#      define ABC_PLATFORM_WINDOWS_PHONE_APP
#    else
//////////////////////////////////////////////////////////////////////////
// WINDOWS
//////////////////////////////////////////////////////////////////////////

#      define ABC_PLATFORM_WINDOWS_DESKTOP_APP
#    endif
#  else
#    define ABC_PLATFORM_WINDOWS_DESKTOP_APP
#  endif

//////////////////////////////////////////////////////////////////////////
// IOS
//////////////////////////////////////////////////////////////////////////

#elif defined(__APPLE__) || defined(MACOSX)
#  include "TargetConditionals.h"

#  define ABC_PLATFORM_POSIX_THREADS_API
#  define ABC_PLATFORM_OSX_FAMILY
#  define ABC_PLATFORM_POSIX_API
#  if (TARGET_IPHONE_SIMULATOR)
#    define ABC_PLATFORM_IOS_FAMILY
#    define ABC_PLATFORM_IOS_SIMULATOR_APP
#  elif (TARGET_OS_IPHONE)
#    define ABC_PLATFORM_IOS_FAMILY
#    define ABC_PLATFORM_IOS_APP
#    if (TARGET_OS_MACCATALYST)
#      define ABC_PLATFORM_CATALYST
#    endif
#  elif (TARGET_OS_MAC)
#    define ABC_PLATFORM_MAC_APP
#  else
#    error "Unknown Platform"
#  endif

//////////////////////////////////////////////////////////////////////////
//LINUX
//////////////////////////////////////////////////////////////////////////

#elif defined(__linux)

#  define ABC_PLATFORM_LINUX_FAMILY
#  define ABC_PLATFORM_LINUX_APP
#  define ABC_PLATFORM_POSIX_API
#  define ABC_PLATFORM_POSIX_THREADS_API

//////////////////////////////////////////////////////////////////////////
// Unknown
//////////////////////////////////////////////////////////////////////////

#else
#  error Unrecognized platform.
#endif
