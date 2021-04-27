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

// template <class T> struct variant_size; // undefined
// template <class T> struct variant_size<const T>;
// template <class T> struct variant_size<volatile T>;
// template <class T> struct variant_size<const volatile T>;
// template <class T> constexpr size_t variant_size_v
//     = variant_size<T>::value;

#include <type_traits>
#include "variant.hpp"

#include "test_macros.h"

template <class V, size_t E> void test() {
  static_assert(swl::variant_size<V>::value == E, "");
  static_assert(swl::variant_size<const V>::value == E, "");
  static_assert(swl::variant_size<volatile V>::value == E, "");
  static_assert(swl::variant_size<const volatile V>::value == E, "");
  static_assert(swl::variant_size_v<V> == E, "");
  static_assert(swl::variant_size_v<const V> == E, "");
  static_assert(swl::variant_size_v<volatile V> == E, "");
  static_assert(swl::variant_size_v<const volatile V> == E, "");
  static_assert(std::is_base_of<std::integral_constant<std::size_t, E>,
                                swl::variant_size<V>>::value,
                "");
};

int main(int, char**) {
  
  // SWL : this is ill-formed, why is this in the test suite? 
  // test<swl::variant<>, 0>();
  test<swl::variant<void *>, 1>();
  test<swl::variant<long, long, void *, double>, 4>();

  return 0;
}
