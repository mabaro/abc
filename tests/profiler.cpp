#include "doctest/doctest.h"

#include "abc/profiler.hpp"

TEST_CASE("abc - error")
{
    using namespace abc;

    ABC_PROFILE_BEGIN("TEST");
    abc::chrono::timer timer;
    ABC_PROFILE_END("TEST");
}
