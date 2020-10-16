#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include "abc/core.hpp"
#include "abc/enum.hpp"

#include "abc/format.hpp"
#include "abc/string.hpp"

#include "abc/tagged_type.hpp"

ABC_ENUM(MyEnum, a, b, c, d);
ABC_ENUM_STRING_IMPL(, MyEnum);

struct A {
	ABC_ENUM(MyEnum, a, b, c, d);
};
ABC_ENUM_STRING_IMPL(A,MyEnum);

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
    }
    {
      CHECK(A::MyEnum::a == A::MyEnum::FIRST);
      CHECK(A::MyEnum::d == A::MyEnum::LAST);
      CHECK(static_cast<size_t>(A::MyEnum::COUNT) == 4);

      CHECK(abc::to_string(A::MyEnum::a) == "a");
      CHECK(abc::to_string(A::MyEnum::b) == "b");
      CHECK(abc::to_string(A::MyEnum::c) == "c");
      CHECK(abc::to_string(A::MyEnum::d) == "d");
    }

    for (MyEnum it = MyEnum::FIRST; it != MyEnum::LAST; it = MyEnum((std::underlying_type<MyEnum>::type)it+1))
	{
      CHECK(abc::to_string(it) == abc::format("{}", it));
    }
	for (A::MyEnum it = A::MyEnum::FIRST; it != A::MyEnum::LAST; it = A::MyEnum((std::underlying_type<A::MyEnum>::type)it + 1))
	{
      CHECK(abc::to_string(it) == abc::format("{}", it));
    }
}

TEST_CASE("abc - tagged_type")
{
    using namespace abc;
	using value_t = float;
	using type1 = tagged_type<value_t, struct tag1, 1>;
	using type2 = tagged_type<value_t, struct tag1, 1>;

	type1 value0(abc::uninitialized);
	type1 value00(abc::uninitialized);

	type1 value1(1);
	type2 value2(2);
	type2 value2a(value2.value() + 1);
	//value2a = value1; // forbidden

	const type1 cvalue1(1);
	const type2 cvalue2(2);
	//cvalue2 = value1; // forbidden

	CHECK(value1.value() == value_t(value1));
	CHECK(value1.value() == (value_t)value1);
	//type2 value2from1(value1);// forbidden implicit conversions

	value2 = type2((value_t)value1);
	value2 = type2(value1.value());
	//value2 = type2(value1);// forbidden implicit conversions

	value1 = cvalue1;
	value2 = type2(value1.value());
	CHECK((value_t)value2 == (value_t)value1);
	CHECK((value_t)cvalue1 == (value_t)value1);

	CHECK((value1 + cvalue1).value() == value1.value() + cvalue1.value());
	CHECK((value1 - cvalue1).value() == value1.value() - cvalue1.value());
	CHECK((value1 * cvalue1).value() == value1.value() * cvalue1.value());
	CHECK((value1 / cvalue1).value() == value1.value() / cvalue1.value());

    CHECK(value1.value() + cvalue1.value() == (value1 += cvalue1).value());
    CHECK(value1.value() - cvalue1.value() == (value1 -= cvalue1).value());
    CHECK(value1.value() * cvalue1.value() == (value1 *= cvalue1).value());
	CHECK(value1.value() / cvalue1.value() == (value1 /= cvalue1).value());
}
