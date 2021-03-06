// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++03, c++11, c++14

// Throwing bad_variant_access is supported starting in macosx10.13
// XFAIL: with_system_cxx_lib=macosx10.12 && !no-exceptions
// XFAIL: with_system_cxx_lib=macosx10.11 && !no-exceptions
// XFAIL: with_system_cxx_lib=macosx10.10 && !no-exceptions
// XFAIL: with_system_cxx_lib=macosx10.9 && !no-exceptions

// <variant>

// template <class ...Types> class variant;

// template <class T, class U, class ...Args>
//   T& emplace(initializer_list<U> il,Args&&... args);

 
#include <string>
#include <type_traits>
#include <swl_assert.hpp> 
#include <swl/variant.hpp> 

#include "archetypes.h"
#include "test_convertible.h"
#include "test_macros.h"

struct InitList {
  std::size_t size;
  constexpr InitList(std::initializer_list<int> il) : size(il.size()) {}
};

struct InitListArg {
  std::size_t size;
  int value;
  constexpr InitListArg(std::initializer_list<int> il, int v)
      : size(il.size()), value(v) {}
};

template <class Var, class T, class... Args>
constexpr auto test_emplace_exists_imp(int) -> decltype(
    std::declval<Var>().template emplace<T>(std::declval<Args>()...), true) {
  return true;
}

template <class, class, class...>
constexpr auto test_emplace_exists_imp(long) -> bool {
  return false;
}

template <class... Args> constexpr bool emplace_exists() {
  return test_emplace_exists_imp<Args...>(0);
}

void test_emplace_sfinae() {
  using V =
      swl::variant<int, TestTypes::NoCtors, InitList, InitListArg, long, long>;
  using IL = std::initializer_list<int>;
  static_assert(emplace_exists<V, InitList, IL>(), "");
  static_assert(!emplace_exists<V, InitList, int>(), "args don't match");
  static_assert(!emplace_exists<V, InitList, IL, int>(), "too many args");
  static_assert(emplace_exists<V, InitListArg, IL, int>(), "");
  static_assert(!emplace_exists<V, InitListArg, int>(), "args don't match");
  static_assert(!emplace_exists<V, InitListArg, IL>(), "too few args");
  static_assert(!emplace_exists<V, InitListArg, IL, int, int>(),
                "too many args");
}

void test_basic() {
  using V = swl::variant<int, InitList, InitListArg, TestTypes::NoCtors>;
  V v;
  auto& ref1 = v.emplace<InitList>({1, 2, 3});
  static_assert(std::is_same_v<InitList&,decltype(ref1)>, "");
  SWL_ASSERT(swl::get<InitList>(v).size == 3);
  SWL_ASSERT(&ref1 == &swl::get<InitList>(v));
  auto& ref2 = v.emplace<InitListArg>({1, 2, 3, 4}, 42);
  static_assert(std::is_same_v<InitListArg&,decltype(ref2)>, "");
  SWL_ASSERT(swl::get<InitListArg>(v).size == 4);
  SWL_ASSERT(swl::get<InitListArg>(v).value == 42);
  SWL_ASSERT(&ref2 == &swl::get<InitListArg>(v));
  auto& ref3 = v.emplace<InitList>({1});
  static_assert(std::is_same_v<InitList&,decltype(ref3)>, "");
  SWL_ASSERT(swl::get<InitList>(v).size == 1);
  SWL_ASSERT(&ref3 == &swl::get<InitList>(v));
}

int main(int, char**) {
  test_basic();
  test_emplace_sfinae();

  SWL_END_TEST_SIGNAL 
 return 0; 

}
