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

// constexpr bool operator<(monostate, monostate) noexcept { return false; }
// constexpr bool operator>(monostate, monostate) noexcept { return false; }
// constexpr bool operator<=(monostate, monostate) noexcept { return true; }
// constexpr bool operator>=(monostate, monostate) noexcept { return true; }
// constexpr bool operator==(monostate, monostate) noexcept { return true; }
// constexpr bool operator!=(monostate, monostate) noexcept { return false; }

#include "test_macros.h"
 
#include <type_traits>
#include <swl_assert.hpp> 
#include <swl/variant.hpp> 

int main(int, char**) {
  using M = swl::monostate;
  constexpr M m1{};
  constexpr M m2{};
  {
    static_assert((m1 < m2) == false, "");
    ASSERT_NOEXCEPT(m1 < m2);
  }
  {
    static_assert((m1 > m2) == false, "");
    ASSERT_NOEXCEPT(m1 > m2);
  }
  {
    static_assert((m1 <= m2) == true, "");
    ASSERT_NOEXCEPT(m1 <= m2);
  }
  {
    static_assert((m1 >= m2) == true, "");
    ASSERT_NOEXCEPT(m1 >= m2);
  }
  {
    static_assert((m1 == m2) == true, "");
    ASSERT_NOEXCEPT(m1 == m2);
  }
  {
    static_assert((m1 != m2) == false, "");
    ASSERT_NOEXCEPT(m1 != m2);
  }

  SWL_END_TEST_SIGNAL 
 return 0; 

}
