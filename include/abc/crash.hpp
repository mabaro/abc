#pragma once

#include "abc/format.hpp"
#include "abc/platform.hpp"

#define ABC_CRASH_MSG(fmt, ...)                                \
    do {                                                       \
        abc::string msg = abc::format(fmt, __VA_ARGS__);       \
        ABC_FAIL(msg);                                         \
        *reinterpret_cast<volatile unsigned int*>(0) = 0xDEAD; \
    } while (false)