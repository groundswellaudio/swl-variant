# swl::variant

A minimal compile-time overhead, C++20 implementation of std::variant. Fully standard conforming with a couple of documented differences. 

## Compile-time performance

Because `std::variant` is implemented in both GCC and Clang libraries using a simple recursive union, accessing each members result in approximately N^2 functions template instantiations for a variant of size N. This implementation instead use a "binary-tree of unions", resulting in N.log2(N) instantiations, which results in drastically faster compile times (see measurements below). 

## Run-time performance and binary size

`std::variant` visit method is usually implemented using a table of functions pointers. Unfortunately, compilers cannot (yet?) "see through" those, and the generated code tends to be much larger and slower than a switch-case equivalent - [more on this here](https://mpark.github.io/programming/2019/01/22/variant-visitation-v2/). Similarly to [Michael Park's implementation](https://github.com/mpark/variant), this implementation use a big, recursive switch for visitation. 

## Testing

The tests come from the LLVM test suite, and are compiled/run by `./tests/all_tests.cpp`.

Compile `all_tests.cpp` as follow : \
`clang++ -std=c++17 ./test/all_tests.cpp`

To run the tests, pass the following arguments to the resulting binary : 
* a string containing a prefix of the command necessary to compile a C++20 file with your compiler of choice... 
* with the root directory and the ./test directory in the include paths... 
* *and* specifying the output path for compilation of individual tests at the end of the command

For example : 
`./a.out clang++ -std=c++20 -I . -I .. -o ./tmp_test`

Some test files succeed by not compiling, so you will see some errors. \
(this is a bit spartan, it would be nice to have a cleaner way of running all the tests). 

## Implementation divergence

* `index()` doesn't return a `std::size_t`, but an integer whose size depends on the numbers of type inside the variant. Basically either `unsigned char` or `unsigned short`. 

* Clang implementation (but not GCC) forbids any kind of conversion to bool, not swl::variant

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

Note : Clang 12 will not work (as in : it works only for trivially destructible types). 
