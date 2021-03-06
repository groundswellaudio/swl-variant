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

//  template <class T, class... Types>
//  constexpr add_pointer_t<T> get_if(variant<Types...>* v) noexcept;
// template <class T, class... Types>
//  constexpr add_pointer_t<const T> get_if(const variant<Types...>* v)
//  noexcept;

#include <swl_assert.hpp> 
#include <swl/variant.hpp> 

#include "test_macros.h"
#include "variant_test_helpers.h"
 

void test_const_get_if() {
  {
    using V = swl::variant<int>;
    constexpr const V *v = nullptr;
    static_assert(swl::get_if<int>(v) == nullptr, "");
  }
  {
    using V = swl::variant<int, const long>;
    constexpr V v(42);
    ASSERT_NOEXCEPT(swl::get_if<int>(&v));
    ASSERT_SAME_TYPE(decltype(swl::get_if<int>(&v)), const int *);
    static_assert(*swl::get_if<int>(&v) == 42, "");
    static_assert(swl::get_if<const long>(&v) == nullptr, "");
  }
  {
    using V = swl::variant<int, const long>;
    constexpr V v(42l);
    ASSERT_SAME_TYPE(decltype(swl::get_if<const long>(&v)), const long *);
    static_assert(*swl::get_if<const long>(&v) == 42, "");
    static_assert(swl::get_if<int>(&v) == nullptr, "");
  }
// FIXME: Remove these once reference support is reinstated
#if !defined(TEST_VARIANT_HAS_NO_REFERENCES)
  {
    using V = swl::variant<int &>;
    int x = 42;
    const V v(x);
    ASSERT_SAME_TYPE(decltype(swl::get_if<int &>(&v)), int *);
    SWL_ASSERT(swl::get_if<int &>(&v) == &x);
  }
  {
    using V = swl::variant<int &&>;
    int x = 42;
    const V v(std::move(x));
    ASSERT_SAME_TYPE(decltype(swl::get_if<int &&>(&v)), int *);
    SWL_ASSERT(swl::get_if<int &&>(&v) == &x);
  }
  {
    using V = swl::variant<const int &&>;
    int x = 42;
    const V v(std::move(x));
    ASSERT_SAME_TYPE(decltype(swl::get_if<const int &&>(&v)), const int *);
    SWL_ASSERT(swl::get_if<const int &&>(&v) == &x);
  }
#endif
}

void test_get_if() {
  {
    using V = swl::variant<int>;
    V *v = nullptr;
    SWL_ASSERT(swl::get_if<int>(v) == nullptr);
  }
  {
    using V = swl::variant<int, const long>;
    V v(42);
    ASSERT_NOEXCEPT(swl::get_if<int>(&v));
    ASSERT_SAME_TYPE(decltype(swl::get_if<int>(&v)), int *);
    SWL_ASSERT(*swl::get_if<int>(&v) == 42);
    SWL_ASSERT(swl::get_if<const long>(&v) == nullptr);
  }
  {
    using V = swl::variant<int, const long>;
    V v(42l);
    ASSERT_SAME_TYPE(decltype(swl::get_if<const long>(&v)), const long *);
    SWL_ASSERT(*swl::get_if<const long>(&v) == 42);
    SWL_ASSERT(swl::get_if<int>(&v) == nullptr);
  }
// FIXME: Remove these once reference support is reinstated
#if !defined(TEST_VARIANT_HAS_NO_REFERENCES)
  {
    using V = swl::variant<int &>;
    int x = 42;
    V v(x);
    ASSERT_SAME_TYPE(decltype(swl::get_if<int &>(&v)), int *);
    SWL_ASSERT(swl::get_if<int &>(&v) == &x);
  }
  {
    using V = swl::variant<const int &>;
    int x = 42;
    V v(x);
    ASSERT_SAME_TYPE(decltype(swl::get_if<const int &>(&v)), const int *);
    SWL_ASSERT(swl::get_if<const int &>(&v) == &x);
  }
  {
    using V = swl::variant<int &&>;
    int x = 42;
    V v(std::move(x));
    ASSERT_SAME_TYPE(decltype(swl::get_if<int &&>(&v)), int *);
    SWL_ASSERT(swl::get_if<int &&>(&v) == &x);
  }
  {
    using V = swl::variant<const int &&>;
    int x = 42;
    V v(std::move(x));
    ASSERT_SAME_TYPE(decltype(swl::get_if<const int &&>(&v)), const int *);
    SWL_ASSERT(swl::get_if<const int &&>(&v) == &x);
  }
#endif
}

int main(int, char**) {
  test_const_get_if();
  test_get_if();

  SWL_END_TEST_SIGNAL 
 return 0; 

}
