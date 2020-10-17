#include "doctest/doctest.h"

#include "abc/core.hpp"
#include "abc/enum.hpp"

#include "abc/format.hpp"
#include "abc/string.hpp"

ABC_ENUM(MyEnum, a, b, c, d);
ABC_ENUM_STRING_IMPL(, MyEnum);

struct A
{
	ABC_ENUM(MyEnum, a, b, c, d);
};
ABC_ENUM_STRING_IMPL(A, MyEnum);

TEST_CASE("abc - enum")
{
	using namespace abc;
	{
		CHECK(MyEnum::a == MyEnum::FIRST);
		CHECK(MyEnum::d == MyEnum::LAST);
		CHECK(static_cast<size_t>(MyEnum::COUNT) == 4);

		CHECK(abc::to_string(MyEnum::a) == "a");
		CHECK(abc::to_string(MyEnum::b) == "b");
		CHECK(abc::to_string(MyEnum::c) == "c");
		CHECK(abc::to_string(MyEnum::d) == "d");
		CHECK(abc::to_string(MyEnum::COUNT) == "COUNT");
	}
	{
		CHECK(A::MyEnum::a == A::MyEnum::FIRST);
		CHECK(A::MyEnum::d == A::MyEnum::LAST);
		CHECK(static_cast<size_t>(A::MyEnum::COUNT) == 4);

		CHECK(abc::to_string(A::MyEnum::a) == "a");
		CHECK(abc::to_string(A::MyEnum::b) == "b");
		CHECK(abc::to_string(A::MyEnum::c) == "c");
		CHECK(abc::to_string(A::MyEnum::d) == "d");
		CHECK(abc::to_string(A::MyEnum::COUNT) == "COUNT");
	}

	for (MyEnum it = MyEnum::FIRST; it != MyEnum::LAST; it = MyEnum(static_cast<std::underlying_type<MyEnum>::type>(it) + 1))
	{
		CHECK(abc::to_string(it) == abc::format("{}", it));
	}
	for (A::MyEnum it = A::MyEnum::FIRST; it != A::MyEnum::LAST; it = A::MyEnum(static_cast<std::underlying_type<A::MyEnum>::type>(it) + 1))
	{
		CHECK(abc::to_string(it) == abc::format("{}", it));
	}
}
