#pragma once

#include "abc/format.hpp"
#include "abc/string.hpp"

#include <vector>

#include <iostream>
#include <sstream>
#include <stdarg.h>   //variadic

namespace abc_compiler {
//////////////////////////////////////////////////////////////////////////

///@brief: use this template to retrieve T's type from compiler
///        Usage: TypeDisplayer<decltype(var_to_investigate)> varType;
template <typename T> class TypeDisplayer;

template <typename T>
void
DisplayType(T param)
{
    ABC_UNUSED(param);
    ABC_FAIL("Needs RTTI");
    // std::cout << "T type: " << typeid(T).name() << std::endl;
    // std::cout << "Param type: " << typeid(param).name() << std::endl;
}

//////////////////////////////////////////////////////////////////////////
}   // namespace abc_compiler

namespace abc {
//////////////////////////////////////////////////////////////////////////

namespace detail {
///////////////////////////////////////////////////////////////////////////////

///@brief compute index of T in the variadic types
template <typename T>
int
ComputeIndexInVariadic(int index = -1)
{
    ABC_UNUSED(index);
    return -1;
}
template <typename T, typename U, typename... Us>
int
ComputeIndexInVariadic(int index = -1)
{
    if (std::is_same<T, U>::value) {
        return index + 1;
    }

    return ComputeIndexInVariadic<T, Us...>(index + 1);
}

///@return variadic argument at index position
template <typename T>
T
get_variadic_argument(int index, const T& param)
{
    ABC_UNUSED(index);
    ABC_ASSERT(index >= 0, "Not enough parameters in the variadic");
    return param;
}
template <typename T, typename... Args>
T
get_variadic_argument(int index, const T& param, Args const&... args)
{
    if (index == 0) {
        return param;
    }
    return get_variadic_argument(index - 1, std::forward<const Args>(args)...);
}

//////////////////////////////////////////////////////////////////////////
}   // namespace detail

namespace algo {
//////////////////////////////////////////////////////////////////////////

template <class String>
std::vector<String>
split(const String& str, const char delimiter)
{
    std::vector<String> result;
    result.reserve(8);

    String accum;
    accum.reserve(8);

    const char* strPtr = str.data();
    while (*strPtr != '\0') {
        if (*strPtr != delimiter) {
            accum.push_back(*strPtr);
        } else {
            if (!accum.empty()) {
                result.push_back(accum);
            }
            accum.clear();
        }

        ++strPtr;
    }

    return result;
}

template <class String>
std::vector<String>
trim(const String& str, const char token)
{
    String accum;
    accum.reserve(str.size());

    const char* strPtr = str.data();
    while (*strPtr != '\0') {
        if (*strPtr != token) {
            accum.push_back(*strPtr);
        }
        ++strPtr;
    }

    accum.shink_to_fit();
    return accum;
}

//////////////////////////////////////////////////////////////////////////
}   // namespace algo

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// is variant/optional really necessary? why not use c++17 standard then? (variants are complex I
// prefer the c++ one)
// In case of willing to extend compatibility, there are many systems (i.e., embedded systems)
// that may not get to c++17 in practical time. For these cases there is need of some replacement.
#if 0
template<typename T, typename...Ts>
struct variant
{
	using this_t = variant<T, Ts...>;

	static const uint8_t k_invalidType = uint8_t(-1);

	static const uint8_t size = 1 + sizeof...(Ts);
	static_assert(sizeof...(Ts) > 0, "Variant type is empty");

	void *m_value = nullptr;
	uint8_t m_type = k_invalidType;

	template<typename S>
	this_t& operator=(S value)
	{
		const uint8_t newType = detail::ComputeIndexInVariadic<S, T, Ts...>();
		ABC_ASSERT_MSG(newType != k_invalidType, "type is not in the variant");

		if (newType != k_invalidType)
		{
			m_type = newType;
			delete m_value;
			m_value = new S(value);
		}

		return *this;
	}

	int get_type_index() const
	{
		return m_type;
	}

	template<typename S>
	bool is() const
	{
		return m_type == detail::ComputeIndexInVariadic<S, T, Ts...>();
	}

	template<typename S>
	const S& get() const
	{
		ABC_ASSERT_MSG(is<S>(), "the value is not of the requested type");
		return *static_cast<S*>(m_value);
	}

	template<typename S>
	S& get()
	{
		ABC_ASSERT_MSG(is<S>(), "the value is not of the requested type");
		return *static_cast<S*>(m_value);
	}
};

#endif

//////////////////////////////////////////////////////////////////////////
}   // namespace abc
