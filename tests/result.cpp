#include "doctest/doctest.h"

#include "abc/core.hpp"
#include "abc/result.hpp"

TEST_CASE("abc - error")
{
    using namespace abc;

    enum class inner1
    {
        a,
        b
    };
    enum class inner2
    {
        a,
        b
    };
    enum class inner3
    {
        a,
        b
    };
    using error_t = error<generic_error_type, error<inner1>, error<inner2, error<inner3>>>;

    auto err0 = error(generic_error_type::generic_error, "generic error");
    CHECK(err0.code() == generic_error_type::generic_error);
    auto err1 = error<inner1>(inner1::a, "inner 1 a");
    CHECK(err1.code() == inner1::a);
    auto err2 = error<inner2, error<inner3>>(inner2::b, "inner 2 b");
    CHECK(err2.code() == inner2::b);
    auto err3 = error<inner2, error<inner3>>(inner2::b, error<inner3>(inner3::b, "inner 3 b"),
                                             "has inner3");
    CHECK(err3.has_inner<error<inner3>>());
    CHECK(err3.inner<error<inner3>>() == inner3::b);

    auto err00 = error_t(err0.code(), err0.message());
    auto err01 = error_t(err0.code(), err1, "has inner1 ");
    auto err02 = error_t(err0.code(), err2, "has inner2 ");
    auto err03 = error_t(err0.code(), err3, "has inner23 ");

    auto msg00 = err00.message_with_inner();
    auto msg01 = err01.message_with_inner();
    auto msg02 = err02.message_with_inner();
    auto msg03 = err03.message_with_inner();

    std::cout << msg00 << std::endl
              << std::endl
              << msg01 << std::endl
              << std::endl
              << msg02 << std::endl
              << std::endl
              << msg03 << std::endl
              << std::endl
              << std::endl;

    CHECK(err00.code() == generic_error_type::generic_error);
    CHECK(err00.code() == generic_error_type::generic_error);
}

template <typename RESULT>
void test_result_for_type()
{
    using namespace abc;

    enum class ErrType
    {
        Type1,
        Type2
    };
    using result_t = result<RESULT, error<ErrType>>;
    using error_t  = typename result_t::error_t;
    static_assert(std::is_same<ErrType, error_t::code_t>::value, "mismatched types");

    const auto successfulResult = RESULT();

    {  // success
        result_t a(successfulResult);
        CHECK_EQ(a, abc::success);
        CHECK_EQ(a.get_payload(), successfulResult);

        CHECK_EQ(result<void>(abc::success), abc::success);
    }

    {  // fails
        result_t b = error_t(ErrType::Type1, abc::string("error 1"));
        CHECK(b != abc::success);
        CHECK(b.get_error().code() == ErrType::Type1);
        CHECK(b.get_error().message() == "error 1");

        result_t b2 = b;
        CHECK(b2 != abc::success);
        CHECK_EQ(b, b2.get_error().code());
        CHECK_EQ(b.get_error().message(), b2.get_error().message());

        result_t b3(successfulResult);
        CHECK_NE(b3, b.get_error().code());

        result_t c = error_t{ErrType::Type2, abc::string("error 2")};
        CHECK(c != abc::success);
        CHECK(c == ErrType::Type2);

        result_t d = c;
        CHECK(d != abc::success);
        CHECK(d == c.get_error().code());
    }
}

TEST_CASE("abc - result")
{
    test_result_for_type<bool>();
    test_result_for_type<int>();
    test_result_for_type<float>();
    test_result_for_type<abc::string>();
    // test_result_for_type<abc::string&>();

    struct Dummy
    {
        int a;
        Dummy() : a(123) {}
        bool operator==(const Dummy& other) const { return other.a == a; }
    };
    test_result_for_type<Dummy>();
}

void t()
{
    struct Dummy
    {
    };
    // abc::result<Dummy&> dummyRefResult;
}