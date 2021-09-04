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

#include <swl_assert.hpp> 
#include <swl/variant.hpp> 

#include "test_macros.h"

struct Incomplete;
template<class T> struct Holder { T t; };

constexpr bool test(bool do_it)
{
    if (do_it) {
        swl::variant<Holder<Incomplete>*, int> v = nullptr;
        swl::visit([](auto){}, v);
        swl::visit([](auto) -> Holder<Incomplete>* { return nullptr; }, v);
#if TEST_STD_VER > 17
        swl::visit<void>([](auto){}, v);
        swl::visit<void*>([](auto) -> Holder<Incomplete>* { return nullptr; }, v);
#endif
    }
    return true;
}

int main(int, char**)
{
    test(true);
#if TEST_STD_VER > 17
    static_assert(test(true));
#endif
    SWL_END_TEST_SIGNAL 
 return 0; 

}
