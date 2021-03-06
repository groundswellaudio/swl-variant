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

// template <class T>
// variant& operator=(T&&) noexcept(see below);

 
#include <string>
#include <type_traits>
#include <swl_assert.hpp> 
#include <swl/variant.hpp> 
#include <memory>

#include "test_macros.h"
#include "variant_test_helpers.h"

namespace MetaHelpers {

struct Dummy {
  Dummy() = default;
};

struct ThrowsCtorT {
  ThrowsCtorT(int) noexcept(false) {}
  ThrowsCtorT &operator=(int) noexcept { return *this; }
};

struct ThrowsAssignT {
  ThrowsAssignT(int) noexcept {}
  ThrowsAssignT &operator=(int) noexcept(false) { return *this; }
};

struct NoThrowT {
  NoThrowT(int) noexcept {}
  NoThrowT &operator=(int) noexcept { return *this; }
};

} // namespace MetaHelpers

namespace RuntimeHelpers {
#ifndef TEST_HAS_NO_EXCEPTIONS

struct ThrowsCtorT {
  int value;
  ThrowsCtorT() : value(0) {}
  ThrowsCtorT(int) noexcept(false) { throw 42; }
  ThrowsCtorT &operator=(int v) noexcept {
    value = v;
    return *this;
  }
};

struct MoveCrashes {
  int value;
  MoveCrashes(int v = 0) noexcept : value{v} {}
  MoveCrashes(MoveCrashes &&) noexcept { SWL_ASSERT(false); }
  MoveCrashes &operator=(MoveCrashes &&) noexcept { SWL_ASSERT(false); return *this; }
  MoveCrashes &operator=(int v) noexcept {
    value = v;
    return *this;
  }
};

struct ThrowsCtorTandMove {
  int value;
  ThrowsCtorTandMove() : value(0) {}
  ThrowsCtorTandMove(int) noexcept(false) { throw 42; }
  ThrowsCtorTandMove(ThrowsCtorTandMove &&) noexcept(false) { SWL_ASSERT(false); }
  ThrowsCtorTandMove &operator=(int v) noexcept {
    value = v;
    return *this;
  }
};

struct ThrowsAssignT {
  int value;
  ThrowsAssignT() : value(0) {}
  ThrowsAssignT(int v) noexcept : value(v) {}
  ThrowsAssignT &operator=(int) noexcept(false) { throw 42; }
};

struct NoThrowT {
  int value;
  NoThrowT() : value(0) {}
  NoThrowT(int v) noexcept : value(v) {}
  NoThrowT &operator=(int v) noexcept {
    value = v;
    return *this;
  }
};

#endif // !defined(TEST_HAS_NO_EXCEPTIONS)
} // namespace RuntimeHelpers

void test_T_assignment_noexcept() {
  using namespace MetaHelpers;
  {
    using V = swl::variant<Dummy, NoThrowT>;
    static_assert(std::is_nothrow_assignable<V, int>::value, "");
  }
  {
    using V = swl::variant<Dummy, ThrowsCtorT>;
    static_assert(!std::is_nothrow_assignable<V, int>::value, "");
  }
  {
    using V = swl::variant<Dummy, ThrowsAssignT>;
    static_assert(!std::is_nothrow_assignable<V, int>::value, "");
  }
}

void test_T_assignment_sfinae() {
  {
    using V = swl::variant<long, long long>;
    static_assert(!std::is_assignable<V, int>::value, "ambiguous");
  }
  {
    using V = swl::variant<std::string, std::string>;
    static_assert(!std::is_assignable<V, const char *>::value, "ambiguous");
  }
  {
    using V = swl::variant<std::string, void *>;
    static_assert(!std::is_assignable<V, int>::value, "no matching operator=");
  }
  {
    using V = swl::variant<std::string, float>;
    static_assert(std::is_assignable<V, int>::value == VariantAllowsNarrowingConversions,
    "no matching operator=");
  }
  {
    using V = swl::variant<std::unique_ptr<int>, bool>;
    static_assert(!std::is_assignable<V, std::unique_ptr<char>>::value,
                  "no explicit bool in operator=");
    struct X {
      operator void*();
    };
    static_assert(!std::is_assignable<V, X>::value,
                  "no boolean conversion in operator=");
	// swl note : unlike libc++ implementation, we allow some boolean conversion
    /* static_assert(!std::is_assignable<V, std::false_type>::value,
                  "no converted to bool in operator="); */
  }
  {
    struct X {};
    struct Y {
      operator X();
    };
    using V = swl::variant<X>;
    static_assert(std::is_assignable<V, Y>::value,
                  "regression on user-defined conversions in operator=");
  }
#if !defined(TEST_VARIANT_HAS_NO_REFERENCES)
  {
    using V = swl::variant<int, int &&>;
    static_assert(!std::is_assignable<V, int>::value, "ambiguous");
  }
  {
    using V = swl::variant<int, const int &>;
    static_assert(!std::is_assignable<V, int>::value, "ambiguous");
  }
#endif // TEST_VARIANT_HAS_NO_REFERENCES
}

void test_T_assignment_basic() {
  {
    swl::variant<int> v(43);
    v = 42;
    SWL_ASSERT(v.index() == 0);
    SWL_ASSERT(swl::get<0>(v) == 42);
  }
  {
    swl::variant<int, long> v(43l);
    v = 42;
    SWL_ASSERT(v.index() == 0);
    SWL_ASSERT(swl::get<0>(v) == 42);
    v = 43l;
    SWL_ASSERT(v.index() == 1);
    SWL_ASSERT(swl::get<1>(v) == 43);
  }
#ifndef TEST_VARIANT_ALLOWS_NARROWING_CONVERSIONS
  {
    swl::variant<unsigned, long> v;
    v = 42;
    SWL_ASSERT(v.index() == 1);
    SWL_ASSERT(swl::get<1>(v) == 42);
    v = 43u;
    SWL_ASSERT(v.index() == 0);
    SWL_ASSERT(swl::get<0>(v) == 43);
  }
#endif
  {
    swl::variant<std::string, bool> v = true;
    v = "bar";
    SWL_ASSERT(v.index() == 0);
    SWL_ASSERT(swl::get<0>(v) == "bar");
  }
  {
    swl::variant<bool, std::unique_ptr<int>> v;
    v = nullptr;
    SWL_ASSERT(v.index() == 1);
    SWL_ASSERT(swl::get<1>(v) == nullptr);
  }
  {
    swl::variant<bool volatile, int> v = 42;
    v = false;
    SWL_ASSERT(v.index() == 0);
    SWL_ASSERT(!swl::get<0>(v));
    bool lvt = true;
    v = lvt;
    SWL_ASSERT(v.index() == 0);
    SWL_ASSERT(swl::get<0>(v));
  }
#if !defined(TEST_VARIANT_HAS_NO_REFERENCES)
  {
    using V = swl::variant<int &, int &&, long>;
    int x = 42;
    V v(43l);
    v = x;
    SWL_ASSERT(v.index() == 0);
    SWL_ASSERT(&swl::get<0>(v) == &x);
    v = std::move(x);
    SWL_ASSERT(v.index() == 1);
    SWL_ASSERT(&swl::get<1>(v) == &x);
    // 'long' is selected by FUN(const int &) since 'const int &' cannot bind
    // to 'int&'.
    const int &cx = x;
    v = cx;
    SWL_ASSERT(v.index() == 2);
    SWL_ASSERT(swl::get<2>(v) == 42);
  }
#endif // TEST_VARIANT_HAS_NO_REFERENCES
}

void test_T_assignment_performs_construction() {
  using namespace RuntimeHelpers;
#ifndef TEST_HAS_NO_EXCEPTIONS
  {
    using V = swl::variant<std::string, ThrowsCtorT>;
    V v(swl::in_place_type<std::string>, "hello");
    try {
      v = 42;
      SWL_ASSERT(false);
    } catch (...) { /* ... */
    }
    SWL_ASSERT(v.index() == 0);
    SWL_ASSERT(swl::get<0>(v) == "hello");
  }
  {
    using V = swl::variant<ThrowsAssignT, std::string>;
    V v(swl::in_place_type<std::string>, "hello");
    v = 42;
    SWL_ASSERT(v.index() == 0);
    SWL_ASSERT(swl::get<0>(v).value == 42);
  }
#endif // TEST_HAS_NO_EXCEPTIONS
}

void test_T_assignment_performs_assignment() {
  using namespace RuntimeHelpers;
#ifndef TEST_HAS_NO_EXCEPTIONS
  {
    using V = swl::variant<ThrowsCtorT>;
    V v;
    v = 42;
    SWL_ASSERT(v.index() == 0);
    SWL_ASSERT(swl::get<0>(v).value == 42);
  }
  {
    using V = swl::variant<ThrowsCtorT, std::string>;
    V v;
    v = 42;
    SWL_ASSERT(v.index() == 0);
    SWL_ASSERT(swl::get<0>(v).value == 42);
  }
  {
    using V = swl::variant<ThrowsAssignT>;
    V v(100);
    try {
      v = 42;
      SWL_ASSERT(false);
    } catch (...) { /* ... */
    }
    SWL_ASSERT(v.index() == 0);
    SWL_ASSERT(swl::get<0>(v).value == 100);
  }
  {
    using V = swl::variant<std::string, ThrowsAssignT>;
    V v(100);
    try {
      v = 42;
      SWL_ASSERT(false);
    } catch (...) { /* ... */
    }
    SWL_ASSERT(v.index() == 1);
    SWL_ASSERT(swl::get<1>(v).value == 100);
  }
#endif // TEST_HAS_NO_EXCEPTIONS
}

int main(int, char**) {
  test_T_assignment_basic();
  test_T_assignment_performs_construction();
  test_T_assignment_performs_assignment();
  test_T_assignment_noexcept();
  test_T_assignment_sfinae();

  SWL_END_TEST_SIGNAL 
 return 0; 

}
