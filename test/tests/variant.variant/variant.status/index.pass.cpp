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

// constexpr size_t index() const noexcept;

 
#include <string>
#include <type_traits>
#include <swl_assert.hpp> 
#include <variant.hpp> 

#include "archetypes.h"
#include "test_macros.h"
#include "variant_test_helpers.h"


int main(int, char**) {
  {
    using V = swl::variant<int, long>;
    constexpr V v;
    static_assert(v.index() == 0, "");
  }
  {
    using V = swl::variant<int, long>;
    V v;
    SWL_ASSERT(v.index() == 0);
  }
  {
    using V = swl::variant<int, long>;
    constexpr V v(swl::in_place_index<1>);
    static_assert(v.index() == 1, "");
  }
  {
    using V = swl::variant<int, std::string>;
    V v("abc");
    SWL_ASSERT(v.index() == 1);
    v = 42;
    SWL_ASSERT(v.index() == 0);
  }
#ifndef TEST_HAS_NO_EXCEPTIONS
  {
    using V = swl::variant<int, MakeEmptyT>;
    V v;
    SWL_ASSERT(v.index() == 0);
    makeEmpty(v);
    SWL_ASSERT(v.index() == swl::variant_npos);
  }
#endif

  SWL_END_TEST_SIGNAL 
 return 0; 

}
