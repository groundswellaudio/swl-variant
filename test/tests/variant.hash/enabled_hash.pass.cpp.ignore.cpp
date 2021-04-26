//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++03, c++11, c++14

// <variant>

// Test that <variant> provides all of the arithmetic, enum, and pointer
// hash specializations.

#include <swl_assert.hpp> 
#include <variant.hpp> 
 

#include "poisoned_hash_helper.h"

#include "test_macros.h"

int main(int, char**) {
  
  // this merely check if <functional> was included? disabling it for now
  test_library_hash_specializations_available();

  SWL_END_TEST_SIGNAL 
 return 0; 

}
