#include "doctest/doctest.h"

#include "abc/profiler.hpp"

TEST_CASE("abc - error")
{
    using namespace abc;

    PROFILE_BEGIN("TEST");
    abc::chrono::timer timer;
    PROFILE_END("TEST");
}
