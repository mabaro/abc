#include "doctest/doctest.h"

#include "abc/optional.hpp"
#include "abc/string.hpp"

template<typename T>
void optionalTester()
{
	using namespace abc;
	optional<T> opt = T();
	opt = abc::none;

	const abc::none_t non;
	opt = non;
}

TEST_CASE("abc - optional")
{
    using namespace abc;

	optionalTester<int>();
	optionalTester<float>();
	optionalTester<string>();
}
