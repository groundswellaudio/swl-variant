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
// XFAIL: with_system_cxx_lib=macosx10.12
// XFAIL: with_system_cxx_lib=macosx10.11
// XFAIL: with_system_cxx_lib=macosx10.10
// XFAIL: with_system_cxx_lib=macosx10.9

// <variant>

/*

 class bad_variant_access : public exception {
public:
  bad_variant_access() noexcept;
  virtual const char* what() const noexcept;
};

*/

 
#include <exception>
#include <type_traits>
#include <swl_assert.hpp> 
#include <variant.hpp> 

#include "test_macros.h"

int main(int, char**) {
  static_assert(std::is_base_of<std::exception, swl::bad_variant_access>::value,
                "");
  static_assert(noexcept(swl::bad_variant_access{"hello"}), "must be noexcept");
  static_assert(noexcept(swl::bad_variant_access{"hello"}.what()), "must be noexcept");
  swl::bad_variant_access ex;
  SWL_ASSERT(ex.what());

  SWL_END_TEST_SIGNAL 
 return 0; 

}
