# swl::variant

A minimal compile-time overhead, C++20 implementation of std::variant. Fully standard conforming with a couple of documented differences. 

## Compile-time performance

Because std::variant is implemented in both GCC and Clang libraries using a simple recursive union, accessing each members result in approximately N^2 functions template instantiations for a variant of size N. This implementation instead use a "binary-tree of unions", resulting in N.log2(N) instantiations, which results in drastically faster compile times (see measurements below). 

## Run-time performance

`std::variant` visit method is usually implemented using a table of functions pointers. Unfortunately, compilers cannot (yet?) "see through" those, and the generated code tends to be much larger and slower than a switch-case equivalent [more on this here](https://mpark.github.io/programming/2019/01/22/variant-visitation-v2/). Similarly to [Michael Park's implementation](https://github.com/mpark/variant), this implementation use a big, recursive switch for visitation. 

## Testing

The tests come from the LLVM libc++ repo. You don't need to use LLVM-lit, though : just compile ./test/all_tests.cpp as follow : 
`clang++ -std=c++20 ./test/all_test.cpp`
To run it, pass a string containing a prefix of the command necessary to compile a C++20 file, with the root directory and the ./test directory in the include paths. 
For example : 
`./a.out clang++ -std=c++20 -I . -I ..`

## Implementation divergence

* Specialization of std::hash is opt-in : specializing std::hash requires including \<functional\>, which is enormous

* `index()` doesn't return a `std::size_t`, but an integer whose size depends on the numbers of type inside the variant. Basically either `unsigned char` or `unsigned short`. 

* Clang implementation (but not GCC) forbids any kind of conversion to bool, not swl::variant

* Extension : if you like to live dangerously, `swl::unsafe_get` behave just like get, but without any errors checking. 

To activate hashing, use the following code before #include'ing : 
`#define SWL_VARIANT_USE_STD_HASH`

## Measurements 

The measurements are of the form (compile time, executable file size). 
All of these measurements were done without optimizations. 

Single visitation : 
| Variant size | swl (clang) | std (clang) | swl (gcc) | std (gcc) 
|--|--|--|--|--|
| 10 | 0.4s   |  1.4s  | 1s  | 1.4s |
| 20 | 0.5s   | 1.4s  | 1.4s | 1.7s |
| 40 | 0.7s   | 2.2s  | 1.8s | 3.2s |
| 80 | 1s     | 5.2s  | 2.6s | 9.2s |
| 160 | 1.7s  | 16.6s | 4.5s | 33.4s |
| 320 | 3.15s | 63s   | 8.4s | 143s |
| 640 | 6.5s  | 315s  | 20s  | ∞ |

Multi visitation of some variants of size 10 : 

| Numbers of variants | swl (clang) | std (clang) | swl (gcc) | std (gcc) 
|--|--|--|--|--|
| 2 | 0.6s    | 2s       | 2.8s, 41 Ko   | 1.3s, 160 Ko  |
| 3 | 2.3s    | 8s       | 3.8s, 123 Ko  | 9s, 1.5 Mo    |
| 4 | 20s     | 80s       | 10.5s, 560 Ko | 95s, 17 Mo    |

### Tested compilers
* GCC 10

