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

// template <class T>
// variant& operator=(T&&) noexcept(see below);

#include <swl_assert.hpp> 
#include <variant.hpp> 
#include <string>
#include <memory>

#include "variant_test_helpers.h"

int main(int, char**)
{
  static_assert(!std::is_assignable<swl::variant<int, int>, int>::value, "");
  static_assert(!std::is_assignable<swl::variant<long, long long>, int>::value, "");
  static_assert(std::is_assignable<swl::variant<char>, int>::value == VariantAllowsNarrowingConversions, "");

  static_assert(std::is_assignable<swl::variant<std::string, float>, int>::value == VariantAllowsNarrowingConversions, "");
  static_assert(std::is_assignable<swl::variant<std::string, double>, int>::value == VariantAllowsNarrowingConversions, "");
  static_assert(!std::is_assignable<swl::variant<std::string, bool>, int>::value, "");

  static_assert(!std::is_assignable<swl::variant<int, bool>, decltype("meow")>::value, "");
  static_assert(!std::is_assignable<swl::variant<int, const bool>, decltype("meow")>::value, "");
  static_assert(!std::is_assignable<swl::variant<int, const volatile bool>, decltype("meow")>::value, "");
  
  // libc++ implementation completely forbids any implicit conversion to bool, not swl::variant
  // static_assert(!std::is_assignable<swl::variant<bool>, std::true_type>::value, ""); 
  static_assert(!std::is_assignable<swl::variant<bool>, std::unique_ptr<char> >::value, "");
  static_assert(!std::is_assignable<swl::variant<bool>, decltype(nullptr)>::value, "");

  SWL_END_TEST_SIGNAL 
 return 0; 

}
