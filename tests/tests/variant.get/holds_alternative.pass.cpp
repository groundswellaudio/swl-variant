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

// template <class T, class... Types>
// constexpr bool holds_alternative(const variant<Types...>& v) noexcept;

#include "test_macros.h"
#include <swl_assert.hpp> 
#include <swl/variant.hpp> 
 

int main(int, char**) {
  {
    using V = swl::variant<int>;
    constexpr V v;
    static_assert(swl::holds_alternative<int>(v), "");
  }
  {
    using V = swl::variant<int, long>;
    constexpr V v;
    static_assert(swl::holds_alternative<int>(v), "");
    static_assert(!swl::holds_alternative<long>(v), "");
  }
  { // noexcept test
    using V = swl::variant<int>;
    const V v;
    ASSERT_NOEXCEPT(swl::holds_alternative<int>(v));
  }

  SWL_END_TEST_SIGNAL 
 return 0; 

}
