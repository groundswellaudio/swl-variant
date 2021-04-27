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

// variant(variant&&) noexcept(see below); // constexpr in C++20

 
#include <string>
#include <type_traits>
#include <swl_assert.hpp> 
#include <variant.hpp> 

#include "test_macros.h"
#include "test_workarounds.h"

struct ThrowsMove {
  ThrowsMove(ThrowsMove &&) noexcept(false) {}
};

struct NoCopy {
  NoCopy(const NoCopy &) = delete;
};

struct MoveOnly {
  int value;
  MoveOnly(int v) : value(v) {}
  MoveOnly(const MoveOnly &) = delete;
  MoveOnly(MoveOnly &&) = default;
};

struct MoveOnlyNT {
  int value;
  MoveOnlyNT(int v) : value(v) {}
  MoveOnlyNT(const MoveOnlyNT &) = delete;
  MoveOnlyNT(MoveOnlyNT &&other) : value(other.value) { other.value = -1; }
};

struct NTMove {
  constexpr NTMove(int v) : value(v) {}
  NTMove(const NTMove &) = delete;
  NTMove(NTMove &&that) : value(that.value) { that.value = -1; }
  int value;
};

static_assert(!std::is_trivially_move_constructible<NTMove>::value, "");
static_assert(std::is_move_constructible<NTMove>::value, "");

struct TMove {
  constexpr TMove(int v) : value(v) {}
  TMove(const TMove &) = delete;
  TMove(TMove &&) = default;
  int value;
};

static_assert(std::is_trivially_move_constructible<TMove>::value, "");

struct TMoveNTCopy {
  constexpr TMoveNTCopy(int v) : value(v) {}
  TMoveNTCopy(const TMoveNTCopy& that) : value(that.value) {}
  TMoveNTCopy(TMoveNTCopy&&) = default;
  int value;
};

static_assert(std::is_trivially_move_constructible<TMoveNTCopy>::value, "");

#ifndef TEST_HAS_NO_EXCEPTIONS
struct MakeEmptyT {
  static int alive;
  MakeEmptyT() { ++alive; }
  MakeEmptyT(const MakeEmptyT &) {
    ++alive;
    // Don't throw from the copy constructor since variant's assignment
    // operator performs a copy before committing to the assignment.
  }
  MakeEmptyT(MakeEmptyT &&) { throw 42; }
  MakeEmptyT &operator=(const MakeEmptyT &) { throw 42; }
  MakeEmptyT &operator=(MakeEmptyT &&) { throw 42; }
  ~MakeEmptyT() { --alive; }
};

int MakeEmptyT::alive = 0;

template <class Variant> void makeEmpty(Variant &v) {
  Variant v2(swl::in_place_type<MakeEmptyT>);
  try {
    v = std::move(v2);
    SWL_ASSERT(false);
  } catch (...) {
    SWL_ASSERT(v.valueless_by_exception());
  }
}
#endif // TEST_HAS_NO_EXCEPTIONS

void test_move_noexcept() {
  {
    using V = swl::variant<int, long>;
    static_assert(std::is_nothrow_move_constructible<V>::value, "");
  }
  {
    using V = swl::variant<int, MoveOnly>;
    static_assert(std::is_nothrow_move_constructible<V>::value, "");
  }
  {
    using V = swl::variant<int, MoveOnlyNT>;
    static_assert(!std::is_nothrow_move_constructible<V>::value, "");
  }
  {
    using V = swl::variant<int, ThrowsMove>;
    static_assert(!std::is_nothrow_move_constructible<V>::value, "");
  }
}

void test_move_ctor_sfinae() {
  {
    using V = swl::variant<int, long>;
    static_assert(std::is_move_constructible<V>::value, "");
  }
  {
    using V = swl::variant<int, MoveOnly>;
    static_assert(std::is_move_constructible<V>::value, "");
  }
  {
    using V = swl::variant<int, MoveOnlyNT>;
    static_assert(std::is_move_constructible<V>::value, "");
  }
  {
    using V = swl::variant<int, NoCopy>;
    static_assert(!std::is_move_constructible<V>::value, "");
  }

  // Make sure we properly propagate triviality (see P0602R4).
#if TEST_STD_VER > 17
  {
    using V = swl::variant<int, long>;
    static_assert(std::is_trivially_move_constructible<V>::value, "");
  }
  {
    using V = swl::variant<int, NTMove>;
    static_assert(!std::is_trivially_move_constructible<V>::value, "");
    static_assert(std::is_move_constructible<V>::value, "");
  }
  {
    using V = swl::variant<int, TMove>;
    static_assert(std::is_trivially_move_constructible<V>::value, "");
  }
  {
    using V = swl::variant<int, TMoveNTCopy>;
    static_assert(std::is_trivially_move_constructible<V>::value, "");
  }
#endif // > C++17
}

template <typename T>
struct Result { size_t index; T value; };

void test_move_ctor_basic() {
  {
    swl::variant<int> v(swl::in_place_index<0>, 42);
    swl::variant<int> v2 = std::move(v);
    SWL_ASSERT(v2.index() == 0);
    SWL_ASSERT(swl::get<0>(v2) == 42);
  }
  {
    swl::variant<int, long> v(swl::in_place_index<1>, 42);
    swl::variant<int, long> v2 = std::move(v);
    SWL_ASSERT(v2.index() == 1);
    SWL_ASSERT(swl::get<1>(v2) == 42);
  }
  {
    swl::variant<MoveOnly> v(swl::in_place_index<0>, 42);
    SWL_ASSERT(v.index() == 0);
    swl::variant<MoveOnly> v2(std::move(v));
    SWL_ASSERT(v2.index() == 0);
    SWL_ASSERT(swl::get<0>(v2).value == 42);
  }
  {
    swl::variant<int, MoveOnly> v(swl::in_place_index<1>, 42);
    SWL_ASSERT(v.index() == 1);
    swl::variant<int, MoveOnly> v2(std::move(v));
    SWL_ASSERT(v2.index() == 1);
    SWL_ASSERT(swl::get<1>(v2).value == 42);
  }
  {
    swl::variant<MoveOnlyNT> v(swl::in_place_index<0>, 42);
    SWL_ASSERT(v.index() == 0);
    swl::variant<MoveOnlyNT> v2(std::move(v));
    SWL_ASSERT(v2.index() == 0);
    SWL_ASSERT(swl::get<0>(v).value == -1);
    SWL_ASSERT(swl::get<0>(v2).value == 42);
  }
  {
    swl::variant<int, MoveOnlyNT> v(swl::in_place_index<1>, 42);
    SWL_ASSERT(v.index() == 1);
    swl::variant<int, MoveOnlyNT> v2(std::move(v));
    SWL_ASSERT(v2.index() == 1);
    SWL_ASSERT(swl::get<1>(v).value == -1);
    SWL_ASSERT(swl::get<1>(v2).value == 42);
  }

  // Make sure we properly propagate triviality, which implies constexpr-ness (see P0602R4).
#if TEST_STD_VER > 17
  {
    struct {
      constexpr Result<int> operator()() const {
        swl::variant<int> v(swl::in_place_index<0>, 42);
        swl::variant<int> v2 = std::move(v);
        return {v2.index(), swl::get<0>(std::move(v2))};
      }
    } test;
    constexpr auto result = test();
    static_assert(result.index == 0, "");
    static_assert(result.value == 42, "");
  }
  {
    struct {
      constexpr Result<long> operator()() const {
        swl::variant<int, long> v(swl::in_place_index<1>, 42);
        swl::variant<int, long> v2 = std::move(v);
        return {v2.index(), swl::get<1>(std::move(v2))};
      }
    } test;
    constexpr auto result = test();
    static_assert(result.index == 1, "");
    static_assert(result.value == 42, "");
  }
  {
    struct {
      constexpr Result<TMove> operator()() const {
        swl::variant<TMove> v(swl::in_place_index<0>, 42);
        swl::variant<TMove> v2(std::move(v));
        return {v2.index(), swl::get<0>(std::move(v2))};
      }
    } test;
    constexpr auto result = test();
    static_assert(result.index == 0, "");
    static_assert(result.value.value == 42, "");
  }
  {
    struct {
      constexpr Result<TMove> operator()() const {
        swl::variant<int, TMove> v(swl::in_place_index<1>, 42);
        swl::variant<int, TMove> v2(std::move(v));
        return {v2.index(), swl::get<1>(std::move(v2))};
      }
    } test;
    constexpr auto result = test();
    static_assert(result.index == 1, "");
    static_assert(result.value.value == 42, "");
  }
  {
    struct {
      constexpr Result<TMoveNTCopy> operator()() const {
        swl::variant<TMoveNTCopy> v(swl::in_place_index<0>, 42);
        swl::variant<TMoveNTCopy> v2(std::move(v));
        return {v2.index(), swl::get<0>(std::move(v2))};
      }
    } test;
    constexpr auto result = test();
    static_assert(result.index == 0, "");
    static_assert(result.value.value == 42, "");
  }
  {
    struct {
      constexpr Result<TMoveNTCopy> operator()() const {
        swl::variant<int, TMoveNTCopy> v(swl::in_place_index<1>, 42);
        swl::variant<int, TMoveNTCopy> v2(std::move(v));
        return {v2.index(), swl::get<1>(std::move(v2))};
      }
    } test;
    constexpr auto result = test();
    static_assert(result.index == 1, "");
    static_assert(result.value.value == 42, "");
  }
#endif // > C++17
}

void test_move_ctor_valueless_by_exception() {
#ifndef TEST_HAS_NO_EXCEPTIONS
  using V = swl::variant<int, MakeEmptyT>;
  V v1;
  makeEmpty(v1);
  V v(std::move(v1));
  SWL_ASSERT(v.valueless_by_exception());
#endif // TEST_HAS_NO_EXCEPTIONS
}

template <size_t Idx>
constexpr bool test_constexpr_ctor_imp(swl::variant<long, void*, const int> const& v) {
  auto copy = v;
  auto v2 = std::move(copy);
  return v2.index() == v.index() &&
         v2.index() == Idx &&
        swl::get<Idx>(v2) == swl::get<Idx>(v);
}

void test_constexpr_move_ctor() {
  // Make sure we properly propagate triviality, which implies constexpr-ness (see P0602R4).
#if TEST_STD_VER > 17
  using V = swl::variant<long, void*, const int>;
#ifdef TEST_WORKAROUND_C1XX_BROKEN_IS_TRIVIALLY_COPYABLE
  static_assert(std::is_trivially_destructible<V>::value, "");
  static_assert(std::is_trivially_copy_constructible<V>::value, "");
  static_assert(std::is_trivially_move_constructible<V>::value, "");
  static_assert(!std::is_copy_assignable<V>::value, "");
  static_assert(!std::is_move_assignable<V>::value, "");
#else // TEST_WORKAROUND_C1XX_BROKEN_IS_TRIVIALLY_COPYABLE
	
  // SWL-FIXME : disabling this for the same reason as the other one in copy test
  // static_assert(std::is_trivially_copyable<V>::value, "");
#endif // TEST_WORKAROUND_C1XX_BROKEN_IS_TRIVIALLY_COPYABLE
  static_assert(std::is_trivially_move_constructible<V>::value, "");
  static_assert(test_constexpr_ctor_imp<0>(V(42l)), "");
  static_assert(test_constexpr_ctor_imp<1>(V(nullptr)), "");
  static_assert(test_constexpr_ctor_imp<2>(V(101)), "");
#endif // > C++17
}


int main(int, char**) {
  test_move_ctor_basic();
  test_move_ctor_valueless_by_exception();
  test_move_noexcept();
  test_move_ctor_sfinae();
  test_constexpr_move_ctor();

  SWL_END_TEST_SIGNAL 
 return 0; 

}
