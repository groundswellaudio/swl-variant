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

// constexpr bool valueless_by_exception() const noexcept;

 
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
    static_assert(!v.valueless_by_exception(), "");
  }
  {
    using V = swl::variant<int, long>;
    V v;
    SWL_ASSERT(!v.valueless_by_exception());
  }
  {
    using V = swl::variant<int, long, std::string>;
    const V v("abc");
    SWL_ASSERT(!v.valueless_by_exception());
  }
#ifndef TEST_HAS_NO_EXCEPTIONS
  {
    using V = swl::variant<int, MakeEmptyT>;
    V v;
    SWL_ASSERT(!v.valueless_by_exception());
    makeEmpty(v);
    SWL_ASSERT(v.valueless_by_exception());
  }
#endif

  SWL_END_TEST_SIGNAL 
 return 0; 

}
