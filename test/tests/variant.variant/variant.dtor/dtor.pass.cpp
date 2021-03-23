// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++03, c++11, c++14

// <variant>

// template <class ...Types> class variant;

// ~variant();

 
#include <type_traits>
#include <swl_assert.hpp> 
#include <variant.hpp> 

#include "test_macros.h"

struct NonTDtor {
  static int count;
  NonTDtor() = default;
  ~NonTDtor() { ++count; }
};
int NonTDtor::count = 0;
static_assert(!std::is_trivially_destructible<NonTDtor>::value, "");

struct NonTDtor1 {
  static int count;
  NonTDtor1() = default;
  ~NonTDtor1() { ++count; }
};
int NonTDtor1::count = 0;
static_assert(!std::is_trivially_destructible<NonTDtor1>::value, "");

struct TDtor {
  TDtor(const TDtor &) {} // non-trivial copy
  ~TDtor() = default;
};
static_assert(!std::is_trivially_copy_constructible<TDtor>::value, "");
static_assert(std::is_trivially_destructible<TDtor>::value, "");

int main(int, char**) {
  {
    using V = swl::variant<int, long, TDtor>;
    static_assert(std::is_trivially_destructible<V>::value, "");
  }
  {
    using V = swl::variant<NonTDtor, int, NonTDtor1>;
    static_assert(!std::is_trivially_destructible<V>::value, "");
    {
      V v(swl::in_place_index<0>);
      SWL_ASSERT(NonTDtor::count == 0);
      SWL_ASSERT(NonTDtor1::count == 0);
    }
    SWL_ASSERT(NonTDtor::count == 1);
    SWL_ASSERT(NonTDtor1::count == 0);
    NonTDtor::count = 0;
    { V v(swl::in_place_index<1>); }
    SWL_ASSERT(NonTDtor::count == 0);
    SWL_ASSERT(NonTDtor1::count == 0);
    {
      V v(swl::in_place_index<2>);
      SWL_ASSERT(NonTDtor::count == 0);
      SWL_ASSERT(NonTDtor1::count == 0);
    }
    SWL_ASSERT(NonTDtor::count == 0);
    SWL_ASSERT(NonTDtor1::count == 1);
  }

  SWL_END_TEST_SIGNAL 
 return 0; 

}
