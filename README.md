# swl::variant

A minimal compile-time overhead, C++20 implementation of std::variant. Fully standard conforming with a couple of documented differences. 

## Compile-time performance

Because `std::variant` is implemented in both GCC and Clang libraries using a simple recursive union, accessing each members result in approximately N^2 functions template instantiations for a variant of size N. This implementation instead use a "binary-tree of unions", resulting in N.log2(N) instantiations, which results in faster compile times (see measurements below). 

## Run-time performance and binary size

`std::variant` visit method is usually implemented using a table of functions pointers. Unfortunately, compilers cannot (yet?) "see through" those, and the generated code tends to be much larger and slower than a switch-case equivalent - [more on this here](https://mpark.github.io/programming/2019/01/22/variant-visitation-v2/). Similarly to [Michael Park's implementation](https://github.com/mpark/variant), this implementation use a big, recursive switch for visitation. 

## Testing

The tests are from the LLVM test suite.
To run them do : 
`mkdir ./test_out && cd ./test_out`
`cmake ../`
`ctest --build-and-test ../ ./ --build-generator "Unix Makefiles"` (replace "Unix Makefiles" as needed)
`make test`

## Implementation divergence

* `index()` doesn't return a `std::size_t`, but an integer whose size depends on the numbers of type inside the variant. Basically either `unsigned char` or `unsigned short`. 
* a `visit_with_index` function is furnished, which is useful for example when you want 
  to apply a function to multiple variant whose index is known to be the same : 
  ```cpp
  visit_with_index(variant1, [&variant2] (auto& elem, auto cst) { foo(elem, get<cst>(variant2)); });
  ```

## Extensions and customization 

* If you like to live dangerously, `swl::unsafe_get` behave just like get, but without any errors checking. 

* Two macro based knobs are available : 
	- `SWL_VARIANT_NO_STD_HASH` : this disable the `std::hash` specializations and avoid the `#include <functional>`, which is big
	- `SWL_VARIANT_NO_CONSTEXPR_EMPLACE` : this disable `constexpr` for emplace, and avoid the `#include <memory>`, which is even bigger. Note that this one is an ODR footgun : don't use it if you can't guarantee that it's enabled everywhere in your binaries. 

	To use these macros, define them in a file named `swl_variant_knobs.hpp`, and put it either in the same directory as `variant.hpp` or at the root of a header search path. 

	Both of these are provided to reduce compile times, whether or not this matter depends on your compiler : on my version of Clang, activating both of these macros result in a mere -0.5s, on GCC however, this reduce compile times by more than 4s. 

## Measurements 

The measurements are of the form (compile time, executable file size). 

All of these measurements were done without optimizations. 

The compilers used were Clang 12 and GCC 10. 

Single visitation : 
| Variant size | swl (clang) | std (clang) | swl (gcc) | std (gcc) 
|--|--|--|--|--|
| 20  | 1s, 50 Ko        | 1.2s, 80 Ko   | 4.6s, 50 Ko  | 1s, 133 Ko   |
| 40  | 1.2s, 120 Ko     | 2s, 260 Ko    | 4.8s, 120 Ko | 2s, 440 Ko   |
| 80  | 1.4s, 300 Ko     | 4.6s, 1 Mo    | 5.3s, 290 Ko | 5.7s, 1.8 Mo |
| 160 | 1.8s, 700 Ko     | 15s, 4.3 Mo   | 6s, 720 Ko   | 21s, 8.2 Mo  |
| 320 | 3s, 1.7 Mo       | 54s, 22 Mo    | 8.4s, 1.8 Mo | 90s, 40 Mo   |
| 640 | 5s, 4 Mo         | 250s, 130 Mo  | 17s, 4.4 Mo  | 415s, 250 Mo |

Multi visitation of some variants of size 10 : 

| Numbers of variants | swl (clang) | std (clang) | swl (gcc) | std (gcc) 
|--|--|--|--|--|
| 2 | 1.1s, 49 Ko   | 1.6s, 128 Ko   | 2.8s, 41 Ko    | 1.3s, 160 Ko  |
| 3 | 2s, 142 Ko    | 8s, 1.1 Mo     | 3.8s, 123 Ko   | 9s, 1.5 Mo    |
| 4 | 6.7s, 630 Ko  | 68s, 11 Mo     | 10.5s, 560 Ko  | 95s, 17 Mo    |

### Tested compilers

* GCC 10

Clang 12/13 will only work for trivially destructible types as their implementation of C++20 is incomplete. 
