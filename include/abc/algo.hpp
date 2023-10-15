#pragma once

#include "abc/function.hpp"
#include <algorithm>
#include <iterator>

namespace abc
{
namespace algo
{
//////////////////////////////////////////////////////////////////////////

template <typename C, typename TValue = typename C::value_type>
auto find(const C& c, const TValue v) -> decltype(std::begin(c)) {
  return std::find(std::begin(c), std::end(c), v);
}

//////////////////////////////////////////////////////////////////////////

template <typename C, typename TValue = typename C::value_type>
auto cfind(const C& c, const TValue& v) -> decltype(std::cbegin(c)) {
  return std::find(std::cbegin(c), std::cend(c), v);
}

//////////////////////////////////////////////////////////////////////////

template <typename C, class UnaryPredicate>
auto find_if(C& c, UnaryPredicate&& pred) -> decltype(std::begin(c)) {
  return std::find_if(std::begin(c), std::end(c), std::forward<UnaryPredicate>(pred));
}

//////////////////////////////////////////////////////////////////////////

template <typename C, class UnaryPredicate>
auto cfind_if(const C& c, UnaryPredicate&& pred) -> decltype(std::cbegin(c)) {
  return std::find_if(std::cbegin(c), std::cend(c), std::forward<UnaryPredicate>(pred));
}

//////////////////////////////////////////////////////////////////////////

template<typename C, typename TValue = typename C::value_type>
bool contains(const C& c, const TValue& v)
{
	return find(c, v) != std::end(c);
}

//////////////////////////////////////////////////////////////////////////

template <typename C, class UnaryPredicate>
bool contains_if(const C& c, UnaryPredicate&& pred) {
  return find_if(c, std::forward<UnaryPredicate>(pred)) != std::end(c);
}

//////////////////////////////////////////////////////////////////////////
}//algo
}//abc
