#include "doctest/doctest.h"

#include "abc/format.hpp"
#include "abc/string.hpp"

TEST_CASE("abc - format") {
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

TEST_CASE("abc - format - int performance") {
  using namespace abc;

  const int32_t samples[] = {
    1,2,3,4,5,6,7,8,9,10,
    11,12,13,14,15,16,17,18,19,20,
    21,22,23,24,25,26,27,28,29,30,
    31,32,33,34,35,36,37,38,39,40,
  };
  const int32_t numSamples = sizeof(samples) / sizeof(samples[0]);

  for (int32_t i = 0; i < 100; ++i) { CHECK(format("{}", to_string(i)) == to_string(i)); }
}

TEST_CASE("abc - format - std::stream int performance") {
  using namespace abc;

  const int32_t samples[] = {
      1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
      21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
  };
  const int32_t numSamples = sizeof(samples) / sizeof(samples[0]);

  for (int32_t i = 0; i < 100; ++i) { CHECK(format("{}", to_string(i)) == to_string(i)); }
}

TEST_CASE("abc - format - str performance") {
  using namespace abc;

  const string samples[] = {
      "abc", "abc", "abc", "abc", "abc", "abc", "abc", "abc", "abc", "abc", "abc", "abc",
      "abc", "abc", "abc", "abc", "abc", "abc", "abc", "abc", "abc", "abc", "abc", "abc",
  };
  const int32_t numSamples = sizeof(samples) / sizeof(samples[0]);

  for (int32_t i = 0; i < 100; ++i) {
    CHECK(format("{}", to_string(samples[i % numSamples])) == to_string(samples[i % numSamples]));
  }
}
