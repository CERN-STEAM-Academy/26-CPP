---
title: Hello SIMD World
author: Dr. Matthias Kretz
date: 0000-03-05
layout: post
---

> TIP
>
> You can do this exercise locally or on Compiler Explorer (unless you want to 
use vir-simd).
{:.block-tip}

Boilerplate:
```c++
#include <experimental/simd>
#include <iostream>

namespace stdx = std::experimental;

template <class T, class A>
std::ostream& operator<<(std::ostream& s, const stdx::simd<T, A>& v) {
    s << '[' << v[0];
    for (std::size_t i = 1; i < v.size(); ++i) {
        s << ", " << v[i];
    }
    return s << ']';
}

int main() {
    return 0;
}
```

> Test simd constructors
>
> Test the four different constructors:
> * default
> * broadcast
> * generator
> * load
>
> ... using different element types: `double`, `char`, `unsigned`, ...
>
> Check what happens if you use a non-vectorizable type.
>
> Test different ABI tags (`simd<T>`, `native_simd<T>`, `fixed_size_simd<T, N>`, 
`simd<T, simd_abi::scalar>`).
{:.block-task}

> Unaligned access
>
> Do an aligned load on an unaligned address.
{:.block-task}

> TIP
>
> You just learned a new reason for SIGSEGV. Remember this when your future self 
stares at the debugger, puzzled how the pointer can be out-of-bounds...
{:.block-tip}

> Implement abs(simd)
>
> Implement and test the absolute value function (not using `stdx::abs`):
> ```c++
> template <class T, class A>
> simd<T, A> abs(simd<T, A> x) {
>   // TODO
> }
> ```
> Note that a correct `std::abs` implementation cares about `-0.`. Bonus points 
if you have an idea...
{:.block-task}

> TIP
>
> * [:green_book: `where_expression`][9]
> * [:green_book: bit operations on floating-point][10]
{:.block-tip}

> Linear search
>
> Given a `std::string_view` (which is a contiguous range of `char`s),
> 1. ... count the number of spaces.
> 2. ... return the index of the first occurrence of a given char.
> 3. ... (optional) return the index of the first occurrence of a given 
substring.
>
> ```c++
> int count_spaces(std::string_view s) {
>   // TODO
> }
>
> int find(std::string_view s, char c) {
>   // TODO
> }
>
> int find(std::string_view s, std::string_view s) {
>   // TODO
> }
> ```
{:.block-task}

> TIP
>
> * [:green_book: `stdx::popcount`][7]
> * [:green_book: `stdx::find_first_set`][8]
{:.block-tip}

> Optional 1: simd_for_each
>
> (A fully general solution of this exercise is part of vir-simd.)
>
> Write a `simd_for_each` algorithm that takes a range and a generic callable:
> ```c++
> template <std::contiguous_range R>
> void simd_for_each(R&& rng, auto&& fun) {
>     // Load simd's from std::ranges::data(rng) and invoke fun with each.
>     // Consider how and when to write back a modified simd.
>     // don't forget the epilogue
> }
> ```
{:.block-task}

> TIP
>
> For a completely generic solution you might want to use:
> * [:green_book: `std::ranges::range_value_t`][1]
> * [:green_book: `std::ranges::contiguous_range`][2]
> * [:green_book: `std::ranges::output_range`][3]
> * [:green_book: `std::invocable`][4]
> * [:green_book: `stdx::resize_simd_t`][5]
{:.block-tip}

> Optional 2: Generalize simd_for_each
>
> Use [`vir::simdize`][6] to generalize your `simd_for_each` from *vectorizable* 
range value types to "simdizable" range value types. I.e. make `simd` iteration 
over array of struct/`std::tuple` easy to use.
>
> This example should work:
> ```c++
> struct Point {
>   float x, y, z;
> };
>
> void normalize(std::vector<Point>& data) {
>   simd_for_each(data, [](auto& v) {
>       auto& [x, y, z] = v;
>       const auto scale = 1.f / sqrt(x * x + y * y + z * z);
>       x *= scale;
>       y *= scale;
>       z *= scale;
>   });
> }
> ```
{:.block-task}

> Bonus: Optimize memory access
>
> Optimize loads and stores: from scalar access to vector access.
{:.block-task}

[1]: https://en.cppreference.com/w/cpp/ranges/iterator_t
[2]: https://en.cppreference.com/w/cpp/ranges/contiguous_range
[3]: https://en.cppreference.com/w/cpp/ranges/output_range
[4]: https://en.cppreference.com/w/cpp/concepts/invocable
[5]: https://en.cppreference.com/w/cpp/experimental/simd/rebind_simd
[6]: https://github.com/mattkretz/vir-simd/#simdize-type-transformation
[7]: https://en.cppreference.com/w/cpp/experimental/simd/popcount
[8]: https://en.cppreference.com/w/cpp/experimental/simd/find_first_set
[9]: https://en.cppreference.com/w/cpp/experimental/simd/where_expression
[10]: https://github.com/mattkretz/vir-simd#bitwise-operators-for-floating-point-simd
