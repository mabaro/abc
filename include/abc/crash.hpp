#pragma once

#include "abc/platform.hpp"
#include "abc/format.hpp"

#define ABC_CRASH_MSG(fmt, ...)                      \
  \                                               
do {                                                 \
	abc::string msg = abc::format(fmt, __VA_ARGS__); \
	ABC_FAIL(msg);                                   \
/*crash_handler();									  */
*reinterpret_cast<volatile unsigned int*>(0) = 0xDEAD;
}
while (false) 