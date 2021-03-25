# swl::variant

A minimal compile-time overhead, C++20 implementation of std::variant. Fully standard conforming with a couple of documented differences. 

## Compile-time performance

Because std::variant is implemented in both GCC and Clang libraries using a simple recursive union, accessing each members result in approximately N^2 functions template instantiations for a variant of size N. This implementation instead use a "binary-tree of unions", resulting in N*log2(N) instantiations. 

Some informal compile time benchmarks : 

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
| 2 | 0.6s | 2s | 1.2s |  2s  |
| 3 | 2.3s | 8s | 4s   | 12s  |
| 4 | 20s | 80s | 35s  | 123s |


## Testing

The tests come from the LLVM libc++ repo. You don't need to use LLVM-lit, though : just compile ./test/all_tests.cpp as follow : 
`clang++ -std=c++20 ./test/all_test.cpp`
To run it, pass a string containing a prefix of the command necessary to compile a C++20 file, with the root directory and the ./test directory in the include paths. 
For example : 
`./a.out clang++ -std=c++20 -I . -I ..`

## Implementation divergence

* there is no constructors/emplace methods that can deduce std::initializer_list
```cpp
swl::variant<std::vector<int>, bool> v { swl::in_place_index<0>, {1, 2, 3} };
// error
swl::variant<std::vector<int>, bool> v { swl::in_place_index<0>, std::initializer_list{1, 2, 3} };
// ok
v.emplace<0>( {1, 2, 3} ); // error
v.emplace<0>( std::initializer_list{1, 2, 3} ); // ok
```
* Specialization of std::hash is opt-in : specializing std::hash requires including \<functional\>, which is enormous

* `index()` doesn't return a `std::size_t`, but an integer whose size depends on the numbers of type inside the variant. Basically either `unsigned char` or `unsigned short`. 

* Clang implementation (but not GCC) forbids any kind of conversion to bool, not swl::variant

To activate hashing, use the following code before #include'ing : 
`#define SWL_VARIANT_USE_STD_HASH`

### Tested compilers
* Clang 11
* GCC 10


