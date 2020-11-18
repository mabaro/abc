#include "doctest/doctest.h"

#include "abc/format.hpp"
#include "abc/enum.hpp"
#include "abc/string.hpp"

TEST_CASE("abc - format - to_string")
{
    using namespace abc;

    CHECK(to_string('t') == "t");
    CHECK(to_string(int(1)) == "1");
    CHECK(to_string(1.3f) == "1.3");
}

TEST_CASE("abc - format - from_string")
{
    using namespace abc;

    CHECK(from_string<char>("1") == '1');
    CHECK(from_string<int>("12") == 12);
    CHECK(from_string<float>("1.2f") == 1.2f);
}

TEST_CASE("abc - format")
{
    using namespace abc;

    CHECK(format("{}", 'h') == "h");
    CHECK(format("{}", int('A')) == "65");
    CHECK(format("{}", "hola") == "hola");
    CHECK(format("{}", string("hola")) == "hola");

    CHECK(format("{}", 1) == "1");
    CHECK(format("{}", 10) == "10");
    CHECK(format("{}", 13.5f) == "13.5");
    CHECK(format("{}", 13.5) == "13.5");

    CHECK(format("{}.{}", 1, 1) == "1.1");
    CHECK(format("{}.{}", 1, 1.5f) == "1.1.5");

    CHECK(format("{}. {}", string("hola"), string("Adios")) == string("hola. Adios"));
}

TEST_CASE("abc - format - to_string int performance")
{
    using namespace abc;
    for (int32_t i = 0; i < 100; ++i)
    {
        CHECK(format("{}", to_string(i)) == to_string(i));
    }
}
