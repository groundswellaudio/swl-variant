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

// template <size_t I, class Up, class ...Args>
// constexpr explicit
// variant(in_place_index_t<I>, initializer_list<Up>, Args&&...);

 
#include <string>
#include <type_traits>
#include <swl_assert.hpp> 
#include <swl/variant.hpp> 

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

void test_ctor_sfinae() {
  using IL = std::initializer_list<int>;
  { // just init list
    using V = swl::variant<InitList, InitListArg, int>;
    static_assert(std::is_constructible<V, swl::in_place_index_t<0>, IL>::value,
                  "");
    static_assert(!test_convertible<V, swl::in_place_index_t<0>, IL>(), "");
  }
  { // too many arguments
    using V = swl::variant<InitList, InitListArg, int>;
    static_assert(
        !std::is_constructible<V, swl::in_place_index_t<0>, IL, int>::value,
        "");
    static_assert(!test_convertible<V, swl::in_place_index_t<0>, IL, int>(),
                  "");
  }
  { // too few arguments
    using V = swl::variant<InitList, InitListArg, int>;
    static_assert(
        !std::is_constructible<V, swl::in_place_index_t<1>, IL>::value, "");
    static_assert(!test_convertible<V, swl::in_place_index_t<1>, IL>(), "");
  }
  { // init list and arguments
    using V = swl::variant<InitList, InitListArg, int>;
    static_assert(
        std::is_constructible<V, swl::in_place_index_t<1>, IL, int>::value, "");
    static_assert(!test_convertible<V, swl::in_place_index_t<1>, IL, int>(),
                  "");
  }
  { // not constructible from arguments
    using V = swl::variant<InitList, InitListArg, int>;
    static_assert(
        !std::is_constructible<V, swl::in_place_index_t<2>, IL>::value, "");
    static_assert(!test_convertible<V, swl::in_place_index_t<2>, IL>(), "");
  }
  { // index not in variant
    using V = swl::variant<InitList, InitListArg, int>;
    static_assert(
        !std::is_constructible<V, swl::in_place_index_t<3>, IL>::value, "");
    static_assert(!test_convertible<V, swl::in_place_index_t<3>, IL>(), "");
  }
}

void test_ctor_basic() {
  {
    constexpr swl::variant<InitList, InitListArg, InitList> v(
        swl::in_place_index<0>, {1, 2, 3});
    static_assert(v.index() == 0, "");
    static_assert(swl::get<0>(v).size == 3, "");
  }
  {
    constexpr swl::variant<InitList, InitListArg, InitList> v(
        swl::in_place_index<2>, {1, 2, 3});
    static_assert(v.index() == 2, "");
    static_assert(swl::get<2>(v).size == 3, "");
  }
  {
    constexpr swl::variant<InitList, InitListArg, InitListArg> v(
        swl::in_place_index<1>, {1, 2, 3, 4}, 42);
    static_assert(v.index() == 1, "");
    static_assert(swl::get<1>(v).size == 4, "");
    static_assert(swl::get<1>(v).value == 42, "");
  }
}

int main(int, char**) {
  test_ctor_basic();
  test_ctor_sfinae();

  SWL_END_TEST_SIGNAL 
 return 0; 

}
