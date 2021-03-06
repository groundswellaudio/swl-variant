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
// XFAIL: with_system_cxx_lib=macosx10.12 && !no-exceptions
// XFAIL: with_system_cxx_lib=macosx10.11 && !no-exceptions
// XFAIL: with_system_cxx_lib=macosx10.10 && !no-exceptions
// XFAIL: with_system_cxx_lib=macosx10.9 && !no-exceptions

// <variant>

// template <class ...Types> class variant;

// template <size_t I, class ...Args>
//   variant_alternative_t<I, variant<Types...>>& emplace(Args&&... args);

 
#include <string>
#include <type_traits>
#include <swl_assert.hpp> 
#include <swl/variant.hpp> 

#include "archetypes.h"
#include "test_convertible.h"
#include "test_macros.h"
#include "variant_test_helpers.h"

template <class Var, size_t I, class... Args>
constexpr auto test_emplace_exists_imp(int) -> decltype(
    std::declval<Var>().template emplace<I>(std::declval<Args>()...), true) {
  return true;
}

template <class, size_t, class...>
constexpr auto test_emplace_exists_imp(long) -> bool {
  return false;
}

template <class Var, size_t I, class... Args> constexpr bool emplace_exists() {
  return test_emplace_exists_imp<Var, I, Args...>(0);
}

void test_emplace_sfinae() {
  {
    using V = swl::variant<int, void *, const void *, TestTypes::NoCtors>;
    static_assert(emplace_exists<V, 0>(), "");
    static_assert(emplace_exists<V, 0, int>(), "");
    static_assert(!emplace_exists<V, 0, decltype(nullptr)>(),
                  "cannot construct");
    static_assert(emplace_exists<V, 1, decltype(nullptr)>(), "");
    static_assert(emplace_exists<V, 1, int *>(), "");
    static_assert(!emplace_exists<V, 1, const int *>(), "");
    static_assert(!emplace_exists<V, 1, int>(), "cannot construct");
    static_assert(emplace_exists<V, 2, const int *>(), "");
    static_assert(emplace_exists<V, 2, int *>(), "");
    static_assert(!emplace_exists<V, 3>(), "cannot construct");
  }
#if !defined(TEST_VARIANT_HAS_NO_REFERENCES)
  {
    using V = swl::variant<int, int &, const int &, int &&, TestTypes::NoCtors>;
    static_assert(emplace_exists<V, 0>(), "");
    static_assert(emplace_exists<V, 0, int>(), "");
    static_assert(emplace_exists<V, 0, long long>(), "");
    static_assert(!emplace_exists<V, 0, int, int>(), "too many args");
    static_assert(emplace_exists<V, 1, int &>(), "");
    static_assert(!emplace_exists<V, 1>(), "cannot default construct ref");
    static_assert(!emplace_exists<V, 1, const int &>(), "cannot bind ref");
    static_assert(!emplace_exists<V, 1, int &&>(), "cannot bind ref");
    static_assert(emplace_exists<V, 2, int &>(), "");
    static_assert(emplace_exists<V, 2, const int &>(), "");
    static_assert(emplace_exists<V, 2, int &&>(), "");
    static_assert(!emplace_exists<V, 2, void *>(),
                  "not constructible from void*");
    static_assert(emplace_exists<V, 3, int>(), "");
    static_assert(!emplace_exists<V, 3, int &>(), "cannot bind ref");
    static_assert(!emplace_exists<V, 3, const int &>(), "cannot bind ref");
    static_assert(!emplace_exists<V, 3, const int &&>(), "cannot bind ref");
    static_assert(!emplace_exists<V, 4>(), "no ctors");
  }
#endif
}

void test_basic() {
  {
    using V = swl::variant<int>;
    V v(42);
    auto& ref1 = v.emplace<0>();
    static_assert(std::is_same_v<int&, decltype(ref1)>, "");
    SWL_ASSERT(swl::get<0>(v) == 0);
    SWL_ASSERT(&ref1 == &swl::get<0>(v));
    auto& ref2 = v.emplace<0>(42);
    static_assert(std::is_same_v<int&, decltype(ref2)>, "");
    SWL_ASSERT(swl::get<0>(v) == 42);
    SWL_ASSERT(&ref2 == &swl::get<0>(v));
  }
  {
    using V =
        swl::variant<int, long, const void *, TestTypes::NoCtors, std::string>;
    const int x = 100;
    V v(swl::in_place_index<0>, -1);
    // default emplace a value
    auto& ref1 = v.emplace<1>();
    static_assert(std::is_same_v<long&, decltype(ref1)>, "");
    SWL_ASSERT(swl::get<1>(v) == 0);
    SWL_ASSERT(&ref1 == &swl::get<1>(v));
    auto& ref2 = v.emplace<2>(&x);
    static_assert(std::is_same_v<const void*&, decltype(ref2)>, "");
    SWL_ASSERT(swl::get<2>(v) == &x);
    SWL_ASSERT(&ref2 == &swl::get<2>(v));
    // emplace with multiple args
    auto& ref3 = v.emplace<4>(3u, 'a');
    static_assert(std::is_same_v<std::string&, decltype(ref3)>, "");
    SWL_ASSERT(swl::get<4>(v) == "aaa");
    SWL_ASSERT(&ref3 == &swl::get<4>(v));
  }
#if !defined(TEST_VARIANT_HAS_NO_REFERENCES)
  {
    using V = swl::variant<int, long, const int &, int &&, TestTypes::NoCtors,
                           std::string>;
    const int x = 100;
    int y = 42;
    int z = 43;
    V v(swl::in_place_index<0>, -1);
    // default emplace a value
    auto& ref1 = v.emplace<1>();
    static_assert(std::is_same_v<long&, decltype(ref1)>, "");
    SWL_ASSERT(swl::get<1>(v) == 0);
    SWL_ASSERT(&ref1 == &swl::get<1>(v));
    // emplace a reference
    auto& ref2 = v.emplace<2>(x);
    static_assert(std::is_same_v<&, decltype(ref)>, "");
    SWL_ASSERT(&swl::get<2>(v) == &x);
    SWL_ASSERT(&ref2 == &swl::get<2>(v));
    // emplace an rvalue reference
    auto& ref3 = v.emplace<3>(std::move(y));
    static_assert(std::is_same_v<&, decltype(ref)>, "");
    SWL_ASSERT(&swl::get<3>(v) == &y);
    SWL_ASSERT(&ref3 == &swl::get<3>(v));
    // re-emplace a new reference over the active member
    auto& ref4 = v.emplace<3>(std::move(z));
    static_assert(std::is_same_v<&, decltype(ref)>, "");
    SWL_ASSERT(&swl::get<3>(v) == &z);
    SWL_ASSERT(&ref4 == &swl::get<3>(v));
    // emplace with multiple args
    auto& ref5 = v.emplace<5>(3u, 'a');
    static_assert(std::is_same_v<std::string&, decltype(ref5)>, "");
    SWL_ASSERT(swl::get<5>(v) == "aaa");
    SWL_ASSERT(&ref5 == &swl::get<5>(v));
  }
#endif
}

int main(int, char**) {
  test_basic();
  test_emplace_sfinae();

  SWL_END_TEST_SIGNAL 
 return 0; 

}
