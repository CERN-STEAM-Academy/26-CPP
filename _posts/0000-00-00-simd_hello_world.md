---
title: Hello SIMD World
author: Dr. Matthias Kretz
date: 0000-03-05
layout: post
---

> TIP
>
> You can do this exercise locally or on Compiler Explorer.
{:.block-tip}

Boilerplate:
```c++
#include <simd>
#include <print>

namespace simd = std::simd;

int main() {
    return 0;
}
```

> Test simd constructors
>
> - Test the four different constructors:
>   * default
>   * broadcast
>   * generator
>   * load (conversion from statically sized range)
> - And test construction via `unchecked_load` and `partial_load`.
>
> ... using different element types: `double`, `char`, `unsigned`, ...
>
> Check what happens if you use a non-vectorizable type.
>
> Test different number of elements.
>
> > Examples
> >
> > ```c++
> > simd::vec<double> v = 1.; // broadcast
> > simd::vec<int> iota([](int i) { return ...; }); // generator
> > simd::vec str = "Hello World"; // CTAD + load constructor
> > ```
> > [C++ Working Draft: [simd.ctor]](https://eel.is/c++draft/simd#ctor)
> {:.block-tip}
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
> Implement and test the absolute value function (not using `simd::abs`):
> ```c++
> template <typename T, typename A>
> constexpr simd::basic_vec<T, A> abs(simd::basic_vecT, A> x) {
>   // TODO
> }
> ```
> Note that a correct `std::abs` implementation cares about `-0.`. Bonus points 
if you have an idea. 😉
{:.block-task}

> TIP
>
> * `simd::select(basic_mask, basic_vec, basic_vec)`
> * [`std::bit_cast`](https://en.cppreference.com/cpp/numeric/bit_cast)
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
> - `simd::reduce_count(basic_mask)`
> - `simd::reduce_min_index(basic_mask)`
{:.block-tip}

> Optional 1: simd_for_each
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
> * [`simd::resize_t<N, V>`](https://eel.is/c++draft/simd.traits)
{:.block-tip}

[1]: https://en.cppreference.com/w/cpp/ranges/iterator_t
[2]: https://en.cppreference.com/w/cpp/ranges/contiguous_range
[3]: https://en.cppreference.com/w/cpp/ranges/output_range
[4]: https://en.cppreference.com/w/cpp/concepts/invocable
