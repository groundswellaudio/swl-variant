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
// template <class Visitor, class... Variants>
// constexpr see below visit(Visitor&& vis, Variants&&... vars);

 
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <swl_assert.hpp> 
#include <variant.hpp> 

#include "test_macros.h"
#include "variant_test_helpers.h"

#include <cstdio>

void test_call_operator_forwarding() {
  using Fn = ForwardingCallObject;
  Fn obj{};
  const Fn &cobj = obj;
  { // test call operator forwarding - no variant
    swl::visit(obj);
    SWL_ASSERT(Fn::check_call<>(CT_NonConst | CT_LValue));
    swl::visit(cobj);
    SWL_ASSERT(Fn::check_call<>(CT_Const | CT_LValue));
    swl::visit(std::move(obj));
    SWL_ASSERT(Fn::check_call<>(CT_NonConst | CT_RValue));
    swl::visit(std::move(cobj));
    SWL_ASSERT(Fn::check_call<>(CT_Const | CT_RValue));
  }
  { // test call operator forwarding - single variant, single arg
    using V = swl::variant<int>;
    V v(42);
    swl::visit(obj, v);
    SWL_ASSERT(Fn::check_call<int &>(CT_NonConst | CT_LValue));
    swl::visit(cobj, v);
    SWL_ASSERT(Fn::check_call<int &>(CT_Const | CT_LValue));
    swl::visit(std::move(obj), v);
    SWL_ASSERT(Fn::check_call<int &>(CT_NonConst | CT_RValue));
    swl::visit(std::move(cobj), v);
    SWL_ASSERT(Fn::check_call<int &>(CT_Const | CT_RValue));
  }
  { // test call operator forwarding - single variant, multi arg
    using V = swl::variant<int, long, double>;
    V v(42l);
    swl::visit(obj, v);
    SWL_ASSERT(Fn::check_call<long &>(CT_NonConst | CT_LValue));
    swl::visit(cobj, v);
    SWL_ASSERT(Fn::check_call<long &>(CT_Const | CT_LValue));
    swl::visit(std::move(obj), v);
    SWL_ASSERT(Fn::check_call<long &>(CT_NonConst | CT_RValue));
    swl::visit(std::move(cobj), v);
    SWL_ASSERT(Fn::check_call<long &>(CT_Const | CT_RValue));
  }
  { // test call operator forwarding - multi variant, multi arg
    using V = swl::variant<int, long, double>;
    using V2 = swl::variant<int *, std::string>;
    V v(42l);
    V2 v2("hello");
    swl::visit(obj, v, v2);
    SWL_ASSERT((Fn::check_call<long &, std::string &>(CT_NonConst | CT_LValue)));
    swl::visit(cobj, v, v2);
    SWL_ASSERT((Fn::check_call<long &, std::string &>(CT_Const | CT_LValue)));
    swl::visit(std::move(obj), v, v2);
    SWL_ASSERT((Fn::check_call<long &, std::string &>(CT_NonConst | CT_RValue)));
    swl::visit(std::move(cobj), v, v2);
    SWL_ASSERT((Fn::check_call<long &, std::string &>(CT_Const | CT_RValue)));
  }
  {
    using V = swl::variant<int, long, double, std::string>;
    V v1(42l), v2("hello"), v3(101), v4(1.1);
    swl::visit(obj, v1, v2, v3, v4);
    SWL_ASSERT((Fn::check_call<long &, std::string &, int &, double &>(CT_NonConst | CT_LValue)));
    swl::visit(cobj, v1, v2, v3, v4);
    SWL_ASSERT((Fn::check_call<long &, std::string &, int &, double &>(CT_Const | CT_LValue)));
    swl::visit(std::move(obj), v1, v2, v3, v4);
    SWL_ASSERT((Fn::check_call<long &, std::string &, int &, double &>(CT_NonConst | CT_RValue)));
    swl::visit(std::move(cobj), v1, v2, v3, v4);
    SWL_ASSERT((Fn::check_call<long &, std::string &, int &, double &>(CT_Const | CT_RValue)));
  }
  {
    using V = swl::variant<int, long, double, int*, std::string>;
    V v1(42l), v2("hello"), v3(nullptr), v4(1.1);
    swl::visit(obj, v1, v2, v3, v4);
    SWL_ASSERT((Fn::check_call<long &, std::string &, int *&, double &>(CT_NonConst | CT_LValue)));
    swl::visit(cobj, v1, v2, v3, v4);
    SWL_ASSERT((Fn::check_call<long &, std::string &, int *&, double &>(CT_Const | CT_LValue)));
    swl::visit(std::move(obj), v1, v2, v3, v4);
    SWL_ASSERT((Fn::check_call<long &, std::string &, int *&, double &>(CT_NonConst | CT_RValue)));
    swl::visit(std::move(cobj), v1, v2, v3, v4);
    SWL_ASSERT((Fn::check_call<long &, std::string &, int *&, double &>(CT_Const | CT_RValue)));
  }
}

void test_argument_forwarding() {
  using Fn = ForwardingCallObject;
  Fn obj{};
  const auto Val = CT_LValue | CT_NonConst;
  { // single argument - value type
    using V = swl::variant<int>;
    V v(42);
    const V &cv = v;
    swl::visit(obj, v);
    SWL_ASSERT(Fn::check_call<int &>(Val));
    swl::visit(obj, cv);
    SWL_ASSERT(Fn::check_call<const int &>(Val));
    swl::visit(obj, std::move(v));
    SWL_ASSERT(Fn::check_call<int &&>(Val));
    swl::visit(obj, std::move(cv));
    SWL_ASSERT(Fn::check_call<const int &&>(Val));
  }
#if !defined(TEST_VARIANT_HAS_NO_REFERENCES)
  { // single argument - lvalue reference
    using V = swl::variant<int &>;
    int x = 42;
    V v(x);
    const V &cv = v;
    swl::visit(obj, v);
    SWL_ASSERT(Fn::check_call<int &>(Val));
    swl::visit(obj, cv);
    SWL_ASSERT(Fn::check_call<int &>(Val));
    swl::visit(obj, std::move(v));
    SWL_ASSERT(Fn::check_call<int &>(Val));
    swl::visit(obj, std::move(cv));
    SWL_ASSERT(Fn::check_call<int &>(Val));
  }
  { // single argument - rvalue reference
    using V = swl::variant<int &&>;
    int x = 42;
    V v(std::move(x));
    const V &cv = v;
    swl::visit(obj, v);
    SWL_ASSERT(Fn::check_call<int &>(Val));
    swl::visit(obj, cv);
    SWL_ASSERT(Fn::check_call<int &>(Val));
    swl::visit(obj, std::move(v));
    SWL_ASSERT(Fn::check_call<int &&>(Val));
    swl::visit(obj, std::move(cv));
    SWL_ASSERT(Fn::check_call<int &&>(Val));
  }
#endif
  { // multi argument - multi variant
    using V = swl::variant<int, std::string, long>;
    V v1(42), v2("hello"), v3(43l);
    swl::visit(obj, v1, v2, v3);
    SWL_ASSERT((Fn::check_call<int &, std::string &, long &>(Val)));
    swl::visit(obj, std::as_const(v1), std::as_const(v2), std::move(v3));
    SWL_ASSERT((Fn::check_call<const int &, const std::string &, long &&>(Val)));
  }
  {
    using V = swl::variant<int, long, double, std::string>;
    V v1(42l), v2("hello"), v3(101), v4(1.1);
    swl::visit(obj, v1, v2, v3, v4);
    SWL_ASSERT((Fn::check_call<long &, std::string &, int &, double &>(Val)));
    swl::visit(obj, std::as_const(v1), std::as_const(v2), std::move(v3), std::move(v4));
    SWL_ASSERT((Fn::check_call<const long &, const std::string &, int &&, double &&>(Val)));
  }
  {
    using V = swl::variant<int, long, double, int*, std::string>;
    V v1(42l), v2("hello"), v3(nullptr), v4(1.1);
    swl::visit(obj, v1, v2, v3, v4);
    SWL_ASSERT((Fn::check_call<long &, std::string &, int *&, double &>(Val)));
    swl::visit(obj, std::as_const(v1), std::as_const(v2), std::move(v3), std::move(v4));
    SWL_ASSERT((Fn::check_call<const long &, const std::string &, int *&&, double &&>(Val)));
  }
}

void test_return_type() {
  using Fn = ForwardingCallObject;
  Fn obj{};
  const Fn &cobj = obj;
  { // test call operator forwarding - no variant
    static_assert(std::is_same_v<decltype(swl::visit(obj)), Fn&>);
    static_assert(std::is_same_v<decltype(swl::visit(cobj)), const Fn&>);
    static_assert(std::is_same_v<decltype(swl::visit(std::move(obj))), Fn&&>);
    static_assert(std::is_same_v<decltype(swl::visit(std::move(cobj))), const Fn&&>);
  }
  { // test call operator forwarding - single variant, single arg
    using V = swl::variant<int>;
    V v(42);
    static_assert(std::is_same_v<decltype(swl::visit(obj, v)), Fn&>);
    static_assert(std::is_same_v<decltype(swl::visit(cobj, v)), const Fn&>);
    static_assert(std::is_same_v<decltype(swl::visit(std::move(obj), v)), Fn&&>);
    static_assert(std::is_same_v<decltype(swl::visit(std::move(cobj), v)), const Fn&&>);
  }
  { // test call operator forwarding - single variant, multi arg
    using V = swl::variant<int, long, double>;
    V v(42l);
    static_assert(std::is_same_v<decltype(swl::visit(obj, v)), Fn&>);
    static_assert(std::is_same_v<decltype(swl::visit(cobj, v)), const Fn&>);
    static_assert(std::is_same_v<decltype(swl::visit(std::move(obj), v)), Fn&&>);
    static_assert(std::is_same_v<decltype(swl::visit(std::move(cobj), v)), const Fn&&>);
  }
  { // test call operator forwarding - multi variant, multi arg
    using V = swl::variant<int, long, double>;
    using V2 = swl::variant<int *, std::string>;
    V v(42l);
    V2 v2("hello");
    static_assert(std::is_same_v<decltype(swl::visit(obj, v, v2)), Fn&>);
    static_assert(std::is_same_v<decltype(swl::visit(cobj, v, v2)), const Fn&>);
    static_assert(std::is_same_v<decltype(swl::visit(std::move(obj), v, v2)), Fn&&>);
    static_assert(std::is_same_v<decltype(swl::visit(std::move(cobj), v, v2)), const Fn&&>);
  }
  {
    using V = swl::variant<int, long, double, std::string>;
    V v1(42l), v2("hello"), v3(101), v4(1.1);
    static_assert(std::is_same_v<decltype(swl::visit(obj, v1, v2, v3, v4)), Fn&>);
    static_assert(std::is_same_v<decltype(swl::visit(cobj, v1, v2, v3, v4)), const Fn&>);
    static_assert(std::is_same_v<decltype(swl::visit(std::move(obj), v1, v2, v3, v4)), Fn&&>);
    static_assert(std::is_same_v<decltype(swl::visit(std::move(cobj), v1, v2, v3, v4)), const Fn&&>);
  }
  {
    using V = swl::variant<int, long, double, int*, std::string>;
    V v1(42l), v2("hello"), v3(nullptr), v4(1.1);
    static_assert(std::is_same_v<decltype(swl::visit(obj, v1, v2, v3, v4)), Fn&>);
    static_assert(std::is_same_v<decltype(swl::visit(cobj, v1, v2, v3, v4)), const Fn&>);
    static_assert(std::is_same_v<decltype(swl::visit(std::move(obj), v1, v2, v3, v4)), Fn&&>);
    static_assert(std::is_same_v<decltype(swl::visit(std::move(cobj), v1, v2, v3, v4)), const Fn&&>);
  }
}

void test_constexpr() {
  constexpr ReturnFirst obj{};
  constexpr ReturnArity aobj{};
  {
    using V = swl::variant<int>;
    constexpr V v(42);
    static_assert(swl::visit(obj, v) == 42, "");
  }
  {
    using V = swl::variant<short, long, char>;
    constexpr V v(42l);
    static_assert(swl::visit(obj, v) == 42, "");
  }
  {
    using V1 = swl::variant<int>;
    using V2 = swl::variant<int, char *, long long>;
    using V3 = swl::variant<bool, int, int>;
    constexpr V1 v1;
    constexpr V2 v2(nullptr);
    constexpr V3 v3;
    static_assert(swl::visit(aobj, v1, v2, v3) == 3, "");
  }
  {
    using V1 = swl::variant<int>;
    using V2 = swl::variant<int, char *, long long>;
    using V3 = swl::variant<void *, int, int>;
    constexpr V1 v1;
    constexpr V2 v2(nullptr);
    constexpr V3 v3;
    static_assert(swl::visit(aobj, v1, v2, v3) == 3, "");
  }
  {
    using V = swl::variant<int, long, double, int *>;
    constexpr V v1(42l), v2(101), v3(nullptr), v4(1.1);
    static_assert(swl::visit(aobj, v1, v2, v3, v4) == 4, "");
  }
  {
    using V = swl::variant<int, long, double, long long, int *>;
    constexpr V v1(42l), v2(101), v3(nullptr), v4(1.1);
    static_assert(swl::visit(aobj, v1, v2, v3, v4) == 4, "");
  }
}

void test_exceptions() {
#ifndef TEST_HAS_NO_EXCEPTIONS
  ReturnArity obj{};
  auto test = [&](auto &&... args) {
    try {
      swl::visit(obj, args...);
    } catch (const swl::bad_variant_access &) {
      return true;
    } catch (...) {
    }
    return false;
  };
  {
    puts("a");
    using V = swl::variant<int, MakeEmptyT>;
    V v;
    makeEmpty(v);
    SWL_ASSERT(test(v));
  }
  {
  	puts("b");
    using V = swl::variant<int, MakeEmptyT>;
    using V2 = swl::variant<long, std::string, void *>;
    V v;
    makeEmpty(v);
    V2 v2("hello");
    SWL_ASSERT(test(v, v2));
  }
  {
  	puts("c");
    using V = swl::variant<int, MakeEmptyT>;
    using V2 = swl::variant<long, std::string, void *>;
    V v;
    makeEmpty(v);
    V2 v2("hello");
    SWL_ASSERT(test(v2, v));
  }
  {
  	puts("d");
    using V = swl::variant<int, MakeEmptyT>;
    using V2 = swl::variant<long, std::string, void *, MakeEmptyT>;
    V v;
    makeEmpty(v);
    V2 v2;
    makeEmpty(v2);
    SWL_ASSERT(test(v, v2));
  }
  {
  	puts("e");
    using V = swl::variant<int, long, double, MakeEmptyT>;
    V v1(42l), v2(101), v3(202), v4(1.1);
    makeEmpty(v1);
    SWL_ASSERT(test(v1, v2, v3, v4));
  }
  {
  	puts("f");
    using V = swl::variant<int, long, double, long long, MakeEmptyT>;
    V v1(42l), v2(101), v3(202), v4(1.1);
    makeEmpty(v1);
    makeEmpty(v2);
    makeEmpty(v3);
    makeEmpty(v4);
    SWL_ASSERT(test(v1, v2, v3, v4));
  }
#endif
}

// See https://llvm.org/PR31916
void test_caller_accepts_nonconst() {
  struct A {};
  struct Visitor {
    void operator()(A&) {}
  };
  swl::variant<A> v;
  swl::visit(Visitor{}, v);
}

int main(int, char**) {
  puts("1");
  test_call_operator_forwarding();
  puts("2");
  test_argument_forwarding();
  puts("3");
  test_return_type();
  puts("4");
  test_constexpr();
  puts("5");
  test_exceptions();
  puts("6");
  test_caller_accepts_nonconst();

  SWL_END_TEST_SIGNAL 
 return 0; 

}
