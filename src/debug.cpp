#include "abc/core.hpp"
#include "abc/debug.hpp"
#include <iostream>

namespace abc
{
namespace assert
{
namespace detail
{

response do_assert(const char* condition, const char* file, int line, const char* message)
{
	std::cerr << "ASSERTION FAILED(" << condition << "): " << message
			  << "[" << file << ":" << line << "]"
			  << std::endl;
	return response::BREAK;
}

}//detail
}//assert
}//abc
