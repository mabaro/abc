#include "abc/enum.hpp"

namespace abc {
namespace detail {
///////////////////////////////////////////////////////////////////////////////

void build_enum_strings(const char* str, abc::string o_strings[], size_t i_expectedStringCount) {
  const char delimiters[] = {',', ' ' };

  size_t ostrIndex = 0;

  abc::string* accum = &o_strings[ostrIndex];
  accum->reserve(8);

  auto isInDelmitersFunc = [&delimiters](const char c) {
    for (const char d : delimiters) {
      if (d == c) return true;
    }
    return false;
  };

  const char* strPtr = str;
  while (*strPtr != '\0' && ostrIndex < i_expectedStringCount) {
    if (!isInDelmitersFunc(*strPtr)) {
      accum->push_back(*strPtr);
    } else {
      if (!accum->empty()) {
        // result.emplace_back(std::move(accum));
        accum = &o_strings[++ostrIndex];
        accum->reserve(8);
      }
    }

    ++strPtr;
  }
}

///////////////////////////////////////////////////////////////////////////////
}  // namespace detail
}  // namespace abc
