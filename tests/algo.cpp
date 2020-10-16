#include "doctest/doctest.h"

#include "abc/core.hpp"
#include "abc/algo.hpp"

#include <vector>

TEST_CASE("abc - algo::find") {
  using namespace abc;

  std::vector<int> v1{1, 2, 3, 4, 5};
  std::vector<int> v2{3, 4, 7, 9, 13};

  CHECK(abc::algo::find(v1, 3) != v1.end());
  CHECK(abc::algo::find(v1, 33) == v1.end());

  int num = 3;
  CHECK(abc::algo::find_if(v1, [num](int i) { return i == num; }) != v1.end());
  CHECK(abc::algo::contains(v1, 3));
  CHECK(!abc::algo::contains(v1, 33));
  CHECK(abc::algo::contains_if(v1, [num](int i) { return i == num; }));

  CHECK(abc::algo::find(v2, 7) != v2.end());
  CHECK(abc::algo::find(v2, 77) == v2.end());
}
