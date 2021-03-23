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

// template <class... Types> struct hash<variant<Types...>>;
// template <> struct hash<monostate>;

 
#include <type_traits>
#include <swl_assert.hpp> 
#include <variant.hpp> 

#include "test_macros.h"
#include "variant_test_helpers.h"
#include "poisoned_hash_helper.h"

#ifndef TEST_HAS_NO_EXCEPTIONS
namespace std {
template <> struct hash<::MakeEmptyT> {
  size_t operator()(const ::MakeEmptyT &) const {
    SWL_ASSERT(false);
    return 0;
  }
};
}
#endif

void test_hash_variant() {
  {
    using V = swl::variant<int, long, int>;
    using H = std::hash<V>;
    const V v(swl::in_place_index<0>, 42);
    const V v_copy = v;
    V v2(swl::in_place_index<0>, 100);
    const H h{};
    SWL_ASSERT(h(v) == h(v));
    SWL_ASSERT(h(v) != h(v2));
    SWL_ASSERT(h(v) == h(v_copy));
    {
      ASSERT_SAME_TYPE(decltype(h(v)), std::size_t);
      static_assert(std::is_copy_constructible<H>::value, "");
    }
  }
  {
    using V = swl::variant<swl::monostate, int, long, const char *>;
    using H = std::hash<V>;
    const char *str = "hello";
    const V v0;
    const V v0_other;
    const V v1(42);
    const V v1_other(100);
    V v2(100l);
    V v2_other(999l);
    V v3(str);
    V v3_other("not hello");
    const H h{};
    SWL_ASSERT(h(v0) == h(v0));
    SWL_ASSERT(h(v0) == h(v0_other));
    SWL_ASSERT(h(v1) == h(v1));
    SWL_ASSERT(h(v1) != h(v1_other));
    SWL_ASSERT(h(v2) == h(v2));
    SWL_ASSERT(h(v2) != h(v2_other));
    SWL_ASSERT(h(v3) == h(v3));
    SWL_ASSERT(h(v3) != h(v3_other));
    SWL_ASSERT(h(v0) != h(v1));
    SWL_ASSERT(h(v0) != h(v2));
    SWL_ASSERT(h(v0) != h(v3));
    SWL_ASSERT(h(v1) != h(v2));
    SWL_ASSERT(h(v1) != h(v3));
    SWL_ASSERT(h(v2) != h(v3));
  }
#ifndef TEST_HAS_NO_EXCEPTIONS
  {
    using V = swl::variant<int, MakeEmptyT>;
    using H = std::hash<V>;
    V v;
    makeEmpty(v);
    V v2;
    makeEmpty(v2);
    const H h{};
    SWL_ASSERT(h(v) == h(v2));
  }
#endif
}

void test_hash_monostate() {
  using H = std::hash<swl::monostate>;
  const H h{};
  swl::monostate m1{};
  const swl::monostate m2{};
  SWL_ASSERT(h(m1) == h(m1));
  SWL_ASSERT(h(m2) == h(m2));
  SWL_ASSERT(h(m1) == h(m2));
  {
    ASSERT_SAME_TYPE(decltype(h(m1)), std::size_t);
    ASSERT_NOEXCEPT(h(m1));
    static_assert(std::is_copy_constructible<H>::value, "");
  }
  {
    test_hash_enabled_for_type<swl::monostate>();
  }
}

void test_hash_variant_duplicate_elements() {
    // Test that the index of the alternative participates in the hash value.
    using V = swl::variant<swl::monostate, swl::monostate>;
    using H = std::hash<V>;
    H h{};
    const V v1(swl::in_place_index<0>);
    const V v2(swl::in_place_index<1>);
    SWL_ASSERT(h(v1) == h(v1));
    SWL_ASSERT(h(v2) == h(v2));
    LIBCPP_ASSERT(h(v1) != h(v2));
}

struct A {};
struct B {};

namespace std {

template <>
struct hash<B> {
  size_t operator()(B const&) const {
	return 0;
  }
};

}

void test_hash_variant_enabled() {

  {
    test_hash_enabled_for_type<swl::variant<int> >();
    test_hash_enabled_for_type<swl::variant<int*, long, double, const int> >();
  }
  {
    test_hash_disabled_for_type<swl::variant<int, A>>();
    test_hash_disabled_for_type<swl::variant<const A, void*>>();
  }
  {
    test_hash_enabled_for_type<swl::variant<int, B>>();
    test_hash_enabled_for_type<swl::variant<const B, int>>();
  }
}


int main(int, char**) {
  test_hash_variant();
  test_hash_variant_duplicate_elements();
  test_hash_monostate();
  test_hash_variant_enabled();

  SWL_END_TEST_SIGNAL 
 return 0; 

}
