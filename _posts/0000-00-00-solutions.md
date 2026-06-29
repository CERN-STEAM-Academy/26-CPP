---
title: (Some) Solutions
author: Dr. Matthias Kretz
date: 2026-06-30
layout: post
---

## Write a benchmark

```c++
#include "benchmark.h"
#include <simd>

namespace simd = std::simd;

template <int N>
void peak(benchmark::State &state)
{
  simd::vec<float, N> x = {};
  benchmark::DoNotOptimize(x);
  for (auto _ : state) {
    x = x * 3.f + 1.f;
  }
  benchmark::DoNotOptimize(x);
  add_flop_counters(state, N * 2);
}

BENCHMARK(peak<1>);
BENCHMARK(peak<2>);
BENCHMARK(peak<4>);
BENCHMARK(peak<8>);
BENCHMARK(peak<16>);
BENCHMARK(peak<32>);
BENCHMARK(peak<48>);
BENCHMARK(peak<64>);
BENCHMARK(peak<80>);
BENCHMARK(peak<96>);
BENCHMARK(peak<112>);
BENCHMARK(peak<128>);
```

### Output

```
Run on (14 X 1700 MHz CPU s)
CPU Caches:
  L1 Data 48 KiB (x4)
  L1 Instruction 64 KiB (x4)
  L2 Unified 2048 KiB (x7)
  L3 Unified 12288 KiB (x1)
Load Average: 0.52, 1.10, 1.01
---------------------------------------------------------------------------------------------------
Benchmark           Time             CPU   Iterations     CYCLES       FLOP FLOP/cycle INSTRUCTIONS
---------------------------------------------------------------------------------------------------
peak<1>          2.71 ns         2.71 ns    295844456    4.45629 737.935M/s   0.448804            2
peak<2>          3.76 ns         3.76 ns    185711643    6.36179  1.0638G/s   0.628754      4.00001
peak<4>          2.86 ns         2.86 ns    295824655    4.72075 2.79428G/s    1.69465            2
peak<8>          2.36 ns         2.36 ns    295923535    4.00152 6.76928G/s    3.99848            2
peak<16>         2.48 ns         2.48 ns    294906860    4.06391 12.9242G/s     7.8742            4
peak<32>         2.37 ns         2.37 ns    295845177    4.00366  26.982G/s    15.9854            6
peak<48>         2.57 ns         2.57 ns    288923995    4.21806 37.3682G/s    22.7593            8
peak<64>         2.59 ns         2.58 ns    270827526    4.37384 49.5366G/s    29.2649           10
peak<80>         3.07 ns         3.07 ns    234843234    5.02873 52.1983G/s    31.8172           12
peak<96>         3.56 ns         3.56 ns    196838009    6.01322 53.8902G/s    31.9296           14
peak<112>        4.14 ns         4.14 ns    158952168    7.00724 54.1191G/s    31.9669           16
peak<128>        5.94 ns         5.94 ns    115103753    10.0482 43.1273G/s    25.4773           24
```


## Hello SIMD World

### `simd_for_each`

```c++
#include <functional>
#include <simd>
#include <span>

namespace simd = std::simd;

// Invokes fun(V&) or fun(const V&) with V copied from the beginning of rg.
// If write_back is true, copy it back to rg.
template <typename V, bool write_back, typename T>
constexpr
void simd_invoke(auto&& fun, std::span<T> rg)
{
  std::conditional_t<write_back, V, const V> chunk = simd::unchecked_load<V>(rg);
  std::invoke(fun, chunk);
  if constexpr (write_back) {
    simd::unchecked_store(chunk, rg);
  }
}

template <class V0, bool write_back, typename T>
constexpr
void simd_for_each_epilogue(auto&& fun, std::span<T> rg)
{
  using V = simd::resize_t<V0::size() / 2, V0>;
  constexpr std::size_t vn = V::size();
  if (vn <= rg.size()) {
    simd_invoke<V, write_back>(fun, rg);
    rg = rg.subspan(vn);
  }
  if constexpr (vn > 1) {
    simd_for_each_epilogue<V, write_back>(fun, rg);
  }
}

template <typename T, std::size_t N, typename F>
constexpr
void simd_for_each(std::span<T, N> rg, F&& fun)
{
  using V = simd::vec<std::remove_const_t<T>>;
  constexpr std::size_t vn = V::size();
  constexpr bool write_back =
      not std::is_const_v<T> and std::invocable<F, V&> and not std::invocable<F, V&&>;
  std::size_t i = 0;
  for (; i + vn <= rg.size(); i += vn) {
    simd_invoke<V, write_back>(fun, rg.subspan(i));
  }
  simd_for_each_epilogue<V, write_back>(fun, rg.subspan(i));
}

// test
consteval
{
  std::vector data = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  simd_for_each(std::span(data), [](auto &x) { x += x; });
  for (int i = 0; i < int(data.size()); ++i)
    if (data[i] != i + i)
      throw i;
}
```

## Benchmark: count spaces

```c++
/* SPDX-License-Identifier: MIT-CMU */
/* Copyright © 2023 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                  Matthias Kretz <m.kretz@gsi.de>
 */

#include <vir/simd.h>
#include <vir/simd_benchmarking.h>
#include <vir/simd_bitset.h>

#include <algorithm>
#include <iostream>
#include <numeric>
#include <ranges>
#include <string_view>

#include "benchmark.h"

namespace stdx = vir::stdx;

// Invokes fun(V&) or fun(const V&) with V copied from rg at offset i.
// If write_back is true, copy it back to rg.
template <typename V, bool write_back>
[[gnu::always_inline]] constexpr
void simd_invoke(auto&& fun, auto&& rg, std::size_t i) {
    std::conditional_t<write_back, V, const V> chunk(std::ranges::data(rg) + i,
                                                     stdx::element_aligned);
    std::invoke(fun, chunk);
    if constexpr (write_back) {
        chunk.copy_to(std::ranges::data(rg) + i, stdx::element_aligned);
    }
}

template <class V0, bool write_back>
[[gnu::always_inline]] constexpr
void simd_for_each_epilogue(auto&& fun, auto&& rg, std::size_t i) {
    using V = stdx::resize_simd_t<V0::size() / 2, V0>;
    if (i + V::size() <= std::ranges::size(rg)) {
        simd_invoke<V, write_back>(fun, rg, i);
        i += V::size();
    }
    if constexpr (V::size() > 1) {
        simd_for_each_epilogue<V, write_back>(fun, rg, i);
    }
}

template <std::ranges::contiguous_range R, typename F>
[[gnu::always_inline]] constexpr
void simd_for_each(R&& rg, F&& fun) {
    using V = stdx::native_simd<std::ranges::range_value_t<R>>;
    constexpr bool write_back =
        std::ranges::output_range<R, typename V::value_type> and
        std::invocable<F, V&> and not std::invocable<F, V&&>;
    std::size_t i = 0;
/*    for (; i + 4 * V::size() <= std::ranges::size(rg); i += 4 * V::size()) {
        simd_invoke<V, write_back>(fun, rg, i);
        simd_invoke<V, write_back>(fun, rg, i + V::size());
        simd_invoke<V, write_back>(fun, rg, i + 2 * V::size());
        simd_invoke<V, write_back>(fun, rg, i + 3 * V::size());
    }*/
    for (; i + V::size() <= std::ranges::size(rg); i += V::size()) {
        simd_invoke<V, write_back>(fun, rg, i);
    }
    simd_for_each_epilogue<V, write_back>(fun, rg, i);
}

std::array<std::string_view, 64> test_strings = {
    "Returns the number of elements in the range [first, last) satisfying specific "
    "criteria.",
    "1) counts the elements that are equal to value.",
    "3) counts elements for which predicate p returns true.",
    "2,4) Same as (1,3), but uses r as the source range, as if using ranges::begin(r) as "
    "first and ranges::end(r) as last.",
    "The function-like entities described on this page are niebloids, that is:",
    "",
    "Explicit template argument lists cannot be specified when calling any of them.\n"
    "None of them are visible to argument-dependent lookup.\n"
    "When any of them are found by normal unqualified lookup as the name to the left of "
    "the function-call operator, argument-dependent lookup is inhibited.\n",
    "In practice, they may be implemented as function objects, or with special compiler "
    "extensions.",
    "Parameters\n"
    "first, last   -   the range of elements to examine\n"
    "r   -   the range of the elements to examine\n"
    "value   -   the value to search for\n"
    "pred  -   predicate to apply to the projected elements\n"
    "proj  -   projection to apply to the elements\n",
    "Return value",
    "Number of elements satisfying the condition.",
    "Complexity",
    "Exactly last - first comparisons and projection.",
    "For the number of elements in the range without any additional criteria, see "
    "std::ranges::distance. ]",
    "The C++ source file is processed by the compiler as if the following phases take "
    "place, in this exact order:",
    "Phase 1",
    "1) The individual bytes of the source code file are mapped (in "
    "implementation-defined manner) to the characters of the basic source character set. "
    "In particular, OS-dependent end-of-line indicators are replaced by newline "
    "characters.",
    "2) The set of source file characters accepted is implementation-defined (since "
    "C++11). Any source file character that cannot be mapped to a character in the basic "
    "source character set is replaced by its universal character name (escaped with "
    "\\u or \\U) or by some implementation-defined form that is handled equivalently.",
    "3) Trigraph sequences are replaced by corresponding single-character "
    "representations.",
    "(until C++17)",
    "(until C++23)",
    "Input files that are a sequence of UTF-8 code units (UTF-8 files) are guaranteed to "
    "be supported. The set of other supported kinds of input files is "
    "implementation-defined. If the set is non-empty, the kind of an input file is "
    "determined in an implementation-defined manner that includes a means of designating "
    "input files as UTF-8 files, independent of their content (recognizing the byte "
    "order mark is not sufficient).",
    "If an input file is determined to be a UTF-8 file, then it shall be a well-formed "
    "UTF-8 code unit sequence and it is decoded to produce a sequence of Unicode scalar "
    "values. A sequence of translation character set elements is then formed by mapping "
    "each Unicode scalar value to the corresponding translation character set element. "
    "In the resulting sequence, each pair of characters in the input sequence consisting "
    "of carriage return (U+000D) followed by line feed (U+000A), as well as each "
    "carriage return (U+000D) not immediately followed by a line feed (U+000A), is "
    "replaced by a single new-line character.",
    "For any other kind of input file supported by the implementation, characters are "
    "mapped (in implementation-defined manner) to a sequence of translation character "
    "set elements. In particular, OS-dependent end-of-line indicators are replaced by "
    "new-line characters.",
    "(since C++23)",
    "Phase 2",
    "1) If the first translation character is byte order mark (U+FEFF), it is deleted. "
    "(since C++23) Whenever backslash appears at the end of a line (immediately followed "
    "by zero or more whitespace characters other than new-line followed by (since C++23) "
    "the newline character), these characters are deleted, combining two physical source "
    "lines into one logical source line. This is a single-pass operation; a line ending "
    "in two backslashes followed by an empty line does not combine three lines into one. "
    "If a universal character name is formed outside raw string literals (since C++11) "
    "in this phase, the behavior is undefined.",
    "2) If a non-empty source file does not end with a newline character after this step "
    "(whether it had no newline originally, or it ended with a newline immediately "
    "preceded by a backslash), a terminating newline character is added.",
    "Unlike static_cast, but like const_cast, the reinterpret_cast expression does not "
    "compile to any CPU instructions (except when converting between integers and "
    "pointers or on obscure architectures where pointer representation depends on its "
    "type). It is purely a compile-time directive which instructs the compiler to treat "
    "expression as if it had the type new-type.",
    "Only the following conversions can be done with reinterpret_cast, except when such "
    "conversions would cast away constness or volatility.",
    "1) An expression of integral, enumeration, pointer, or pointer-to-member type can "
    "be converted to its own type. The resulting value is the same as the value of "
    "expression.",
    "2) A pointer can be converted to any integral type large enough to hold all values "
    "of its type (e.g. to std::uintptr_t)",
    "3) A value of any integral or enumeration type can be converted to a pointer type. "
    "A pointer converted to an integer of sufficient size and back to the same pointer "
    "type is guaranteed to have its original value, otherwise the resulting pointer "
    "cannot be dereferenced safely (the round-trip conversion in the opposite direction "
    "is not guaranteed; the same pointer may have multiple integer representations) The "
    "null pointer constant NULL or integer zero is not guaranteed to yield the null "
    "pointer value of the target type; static_cast or implicit conversion should be used "
    "for this purpose.",
    "4) Any value of type std::nullptr_t, including nullptr can be converted to any "
    "integral type as if it were (void*)0, but no value, not even nullptr can be "
    "converted to std::nullptr_t: static_cast should be used for that purpose.",
    "(since C++11)",
    "5) Any object pointer type T1* can be converted to another object pointer type cv "
    "T2*. This is exactly equivalent to static_cast<cv T2*>(static_cast<cv "
    "void*>(expression)) (which implies that if T2's alignment requirement is not "
    "stricter than T1's, the value of the pointer does not change and conversion of the "
    "resulting pointer back to its original type yields the original value). In any "
    "case, the resulting pointer may only be dereferenced safely if allowed by the type "
    "aliasing rules (see below)",
    "6) An lvalue (until C++11)glvalue (since C++11) expression of type T1 can be "
    "converted to reference to another type T2. The result is that of "
    "*reinterpret_cast<T2*>(p), where p is a pointer of type “pointer to T1” to the "
    "object designated by expression. No temporary is created, no copy is made, no "
    "constructors or conversion functions are called. The resulting reference can only "
    "be accessed safely if allowed by the type aliasing rules (see below)",
    "7) Any pointer to function can be converted to a pointer to a different function "
    "type. Calling the function through a pointer to a different function type is "
    "undefined, but converting such pointer back to pointer to the original function "
    "type yields the pointer to the original function.",
    "8) On some implementations (in particular, on any POSIX compatible system as "
    "required by dlsym), a function pointer can be converted to void* or any other "
    "object pointer, or vice versa. If the implementation supports conversion in both "
    "directions, conversion to the original type yields the original value, otherwise "
    "the resulting pointer cannot be dereferenced or called safely.",
    "9) The null pointer value of any pointer type can be converted to any other pointer "
    "type, resulting in the null pointer value of that type. Note that the null pointer "
    "constant nullptr or any other value of type std::nullptr_t cannot be converted to a "
    "pointer with reinterpret_cast: implicit conversion or static_cast should be used "
    "for this purpose.",
    "10) A pointer to member function can be converted to pointer to a different member "
    "function of a different type. Conversion back to the original type yields the "
    "original value, otherwise the resulting pointer cannot be used safely.",
    "11) A pointer to member object of some class T1 can be converted to a pointer to "
    "another member object of another class T2. If T2's alignment is not stricter than "
    "T1's, conversion back to the original type T1 yields the original value, otherwise "
    "the resulting pointer cannot be used safely.",
    "Many compilers issue \"strict aliasing\" warnings in such cases, even though "
    "technically such constructs run afoul of something other than the paragraph "
    "commonly known as the \"strict aliasing rule\".",
    "The purpose of strict aliasing and related rules is to enable type-based alias "
    "analysis, which would be decimated if a program can validly create a situation "
    "where two pointers to unrelated types (e.g., an int* and a float*) could "
    "simultaneously exist and both can be used to load or store the same memory (see "
    "this email on SG12 reflector). Thus, any technique that is seemingly capable of "
    "creating such a situation necessarily invokes undefined behavior.",
    "When it is needed to interpret the bytes of an object as a value of a different "
    "type, std::memcpy or std::bit_cast (since C++20) can be used: ",
    "If the implementation provides std::intptr_t and/or std::uintptr_t, then a cast "
    "from a pointer to an object type or cv void to these types is always well-defined. "
    "However, this is not guaranteed for a function pointer.",
    "(since C++11)",
    "The paragraph defining the strict aliasing rule in the standard used to contain two "
    "additional bullets partially inherited from C:",
    "AliasedType is an aggregate type or a union type which holds one of the "
    "aforementioned types as an element or non-static member (including, recursively, "
    "elements of subaggregates and non-static data members of the contained unions).",
    "AliasedType is a (possibly cv-qualified) base class of DynamicType.",
    "These bullets describe situations that cannot arise in C++ and therefore are "
    "omitted from the discussion above. In C, aggregate copy and assignment access the "
    "aggregate object as a whole. But in C++ such actions are always performed through a "
    "member function call, which accesses the individual subobjects rather than the "
    "entire object (or, in the case of unions, copies the object representation, i.e., "
    "via unsigned char). These bullets were eventually removed via CWG issue 2051. ",
    "std::memcpy may be used to implicitly create objects in the destination buffer.",
    "std::memcpy is meant to be the fastest library routine for memory-to-memory copy. "
    "It is usually more efficient than std::strcpy, which must scan the data it copies "
    "or std::memmove, which must take precautions to handle overlapping inputs.",
    "Several C++ compilers transform suitable memory-copying loops to std::memcpy calls.",
    "Where strict aliasing prohibits examining the same memory as values of two "
    "different types, std::memcpy may be used to convert the values.",
    "If Derived is polymorphic, such pointer may be used to make virtual function calls.",
    "Certain addition, subtraction, increment, and decrement operators are defined for "
    "pointers to elements of arrays: such pointers satisfy the "
    "LegacyRandomAccessIterator requirements and allow the C++ library algorithms to "
    "work with raw arrays.",
    "Comparison operators are defined for pointers to objects in some situations: two "
    "pointers that represent the same address compare equal, two null pointer values "
    "compare equal, pointers to elements of the same array compare the same as the array "
    "indexes of those elements, and pointers to non-static data members with the same "
    "member access compare in order of declaration of those members.",
    "Many implementations also provide strict total ordering of pointers of random "
    "origin, e.g. if they are implemented as addresses within continuous virtual address "
    "space. Those implementations that do not (e.g. where not all bits of the pointer "
    "are part of a memory address and have to be ignored for comparison, or an "
    "additional calculation is required or otherwise pointer and integer is not a 1 to 1 "
    "relationship), provide a specialization of std::less for pointers that has that "
    "guarantee. This makes it possible to use all pointers of random origin as keys in "
    "standard associative containers such as std::set or std::map.",
    "If the original pointer is pointing to a base class subobject within an object of "
    "some polymorphic type, dynamic_cast may be used to obtain a void* that is pointing "
    "at the complete object of the most derived type.",
    "Pointers to void have the same size, representation and alignment as pointers to "
    "char.",
    "Pointers to void are used to pass objects of unknown type, which is common in C "
    "interfaces: std::malloc returns void*, std::qsort expects a user-provided callback "
    "that accepts two const void* arguments. pthread_create expects a user-provided "
    "callback that accepts and returns void*. In all cases, it is the caller's "
    "responsibility to cast the pointer to the correct type before use.",
    "Pointers to functions",
};

static void add_string_rates(benchmark::State& state) {
  state.counters["rate"] = {test_strings.size(),
                            benchmark::Counter::kIsIterationInvariantRate};
  if (state.counters.contains("CYCLES")) {
    state.counters["cycles/string"] = {
        test_strings.size() / state.counters["CYCLES"],
        benchmark::Counter::kIsIterationInvariant | benchmark::Counter::kInvert};
  }
  state.SetBytesProcessed(state.iterations() *
                          std::accumulate(test_strings.begin(), test_strings.end(), 0uz,
                                          [](std::size_t acc, std::string_view s) {
                                            return acc + s.size();
                                          }));
}

template <typename V, typename Flags>
[[gnu::always_inline]] constexpr
int count_character(char c, const char* ptr, std::size_t len, Flags f)
{
  const std::bitset<V::size()> bits((1ull << len) - 1);
  const auto k = vir::to_simd_mask<char>(bits);
  V chunk = {};
  where(k, chunk).copy_from(ptr, f);
  return popcount(chunk == c);
}

constexpr int count_character(std::string_view s, char c) {
  using V = stdx::native_simd<char>;
  int sum = 0;
  simd_for_each(s, [&](auto chunk) { sum += popcount(chunk == c); });
  return sum;
}

constexpr int count_character_opt(std::string_view s, char c) {
  using V = stdx::native_simd<char>;
  if (s.size() <= V::size()) {
    return count_character<V>(c, s.data(), s.size(), stdx::element_aligned);
  }
  int sum = 0;
  std::size_t i = 0;
  const std::size_t misaligned =
      reinterpret_cast<std::uintptr_t>(s.data()) & (stdx::memory_alignment_v<V> - 1);
  if (misaligned > 0) {
    sum = count_character<V>(c, s.data(), V::size() - misaligned, stdx::element_aligned);
    i = V::size() - misaligned;
  }
  for (; i + V::size() <= s.size(); i += V::size()) {
    sum += popcount(V(s.data() + i, stdx::vector_aligned) == c);
  }
  sum += count_character<V>(c, s.data() + i, s.size() - i, stdx::vector_aligned);
  return sum;
}

constexpr int count_character_opt2(std::string_view s, char c) {
  using V = stdx::native_simd<char>;
  int sum = 0;
  std::size_t i = 0;
  for (; i + V::size() <= s.size(); i += V::size()) {
    sum += popcount(V(s.data() + i, stdx::element_aligned) == c);
  }
  sum += count_character<V>(c, s.data() + i, s.size() - i, stdx::element_aligned);
  return sum;
}

void bench_ranges_count(benchmark::State &state) {
  for (auto _ : state) {
    for (auto s : test_strings) {
      vir::fake_read(std::ranges::count(s, ' '));
    }
  }
  add_string_rates(state);
}

void bench_count_spaces_simd(benchmark::State &state) {
  for (auto _ : state) {
    for (auto s : test_strings) {
      vir::fake_read(count_character(s, ' '));
    }
  }
  for (auto s : test_strings) {
    if (std::ranges::count(s, ' ') != count_character(s, ' ')) {
      std::cerr << "incorrect answer (size = " << s.size()
                << ", addr = " << static_cast<const void*>(s.data()) << "):\n"
                << s << "\n  ranges::count: " << std::ranges::count(s, ' ')
                << " != " << count_character(s, ' ') << " :simd count\n";
    }
  }
  add_string_rates(state);
}

void bench_count_spaces_simd_opt(benchmark::State &state) {
  for (auto _ : state) {
    for (auto s : test_strings) {
      vir::fake_read(count_character_opt(s, ' '));
    }
  }
  for (auto s : test_strings) {
    if (std::ranges::count(s, ' ') != count_character_opt(s, ' ')) {
      std::cerr << "incorrect answer (size = " << s.size()
                << ", addr = " << static_cast<const void*>(s.data()) << "):\n"
                << s << "\n  ranges::count: " << std::ranges::count(s, ' ')
                << " != " << count_character_opt(s, ' ') << " :simd count\n";
    }
  }
  add_string_rates(state);
}

void bench_count_spaces_simd_opt2(benchmark::State &state) {
  for (auto _ : state) {
    for (auto s : test_strings) {
      vir::fake_read(count_character_opt2(s, ' '));
    }
  }
  for (auto s : test_strings) {
    if (std::ranges::count(s, ' ') != count_character_opt2(s, ' ')) {
      std::cerr << "incorrect answer (size = " << s.size()
                << ", addr = " << static_cast<const void*>(s.data()) << "):\n"
                << s << "\n  ranges::count: " << std::ranges::count(s, ' ')
                << " != " << count_character_opt(s, ' ') << " :simd count\n";
    }
  }
  add_string_rates(state);
}

BENCHMARK(bench_ranges_count);
BENCHMARK(bench_count_spaces_simd);
BENCHMARK(bench_count_spaces_simd_opt);
BENCHMARK(bench_count_spaces_simd_opt2);
```

### Output

```
-----------------------------------------------------------------------------------------------------------------------------------------
Benchmark                             Time             CPU   Iterations     CYCLES INSTRUCTIONS bytes_per_second cycles/string       rate
-----------------------------------------------------------------------------------------------------------------------------------------
bench_ranges_count                 2081 ns         2081 ns       337387   9.01987k       18.81k       5.71932G/s       140.935 30.7486M/s
bench_count_spaces_simd             364 ns          364 ns      1916998   1.42147k       4.942k       32.6812G/s       22.2105 175.703M/s
bench_count_spaces_simd_opt         270 ns          270 ns      2595573    1053.32       3.679k       44.0897G/s       16.4581 237.038M/s
bench_count_spaces_simd_opt2        244 ns          244 ns      2857268        950       3.276k         48.86G/s       14.8438 262.685M/s
```
