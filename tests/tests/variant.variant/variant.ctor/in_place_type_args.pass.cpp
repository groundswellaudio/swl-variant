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

// template <class Tp, class ...Args>
// constexpr explicit variant(in_place_type_t<Tp>, Args&&...);

 
#include <type_traits>
#include <swl_assert.hpp> 
#include <swl/variant.hpp> 

#include "test_convertible.h"
#include "test_macros.h"

void test_ctor_sfinae() {
  {
    using V = swl::variant<int>;
    static_assert(
        std::is_constructible<V, swl::in_place_type_t<int>, int>::value, "");
    static_assert(!test_convertible<V, swl::in_place_type_t<int>, int>(), "");
  }
  {
    using V = swl::variant<int, long, long long>;
    static_assert(
        std::is_constructible<V, swl::in_place_type_t<long>, int>::value, "");
    static_assert(!test_convertible<V, swl::in_place_type_t<long>, int>(), "");
  }
  {
    using V = swl::variant<int, long, int *>;
    static_assert(
        std::is_constructible<V, swl::in_place_type_t<int *>, int *>::value,
        "");
    static_assert(!test_convertible<V, swl::in_place_type_t<int *>, int *>(),
                  "");
  }
  { // duplicate type
    using V = swl::variant<int, long, int>;
    static_assert(
        !std::is_constructible<V, swl::in_place_type_t<int>, int>::value, "");
    static_assert(!test_convertible<V, swl::in_place_type_t<int>, int>(), "");
  }
  { // args not convertible to type
    using V = swl::variant<int, long, int *>;
    static_assert(
        !std::is_constructible<V, swl::in_place_type_t<int>, int *>::value, "");
    static_assert(!test_convertible<V, swl::in_place_type_t<int>, int *>(), "");
  }
  { // type not in variant
    using V = swl::variant<int, long, int *>;
    static_assert(
        !std::is_constructible<V, swl::in_place_type_t<long long>, int>::value,
        "");
    static_assert(!test_convertible<V, swl::in_place_type_t<long long>, int>(),
                  "");
  }
}

void test_ctor_basic() {
  {
    constexpr swl::variant<int> v(swl::in_place_type<int>, 42);
    static_assert(v.index() == 0, "");
    static_assert(swl::get<0>(v) == 42, "");
  }
  {
    constexpr swl::variant<int, long> v(swl::in_place_type<long>, 42);
    static_assert(v.index() == 1, "");
    static_assert(swl::get<1>(v) == 42, "");
  }
  {
    constexpr swl::variant<int, const int, long> v(
        swl::in_place_type<const int>, 42);
    static_assert(v.index() == 1, "");
    static_assert(swl::get<1>(v) == 42, "");
  }
  {
    using V = swl::variant<const int, volatile int, int>;
    int x = 42;
    V v(swl::in_place_type<const int>, x);
    SWL_ASSERT(v.index() == 0);
    SWL_ASSERT(swl::get<0>(v) == x);
  }
  {
    using V = swl::variant<const int, volatile int, int>;
    int x = 42;
    V v(swl::in_place_type<volatile int>, x);
    SWL_ASSERT(v.index() == 1);
    SWL_ASSERT(swl::get<1>(v) == x);
  }
  {
    using V = swl::variant<const int, volatile int, int>;
    int x = 42;
    V v(swl::in_place_type<int>, x);
    SWL_ASSERT(v.index() == 2);
    SWL_ASSERT(swl::get<2>(v) == x);
  }
}

int main(int, char**) {
  test_ctor_basic();
  test_ctor_sfinae();

  SWL_END_TEST_SIGNAL 
 return 0; 

}
