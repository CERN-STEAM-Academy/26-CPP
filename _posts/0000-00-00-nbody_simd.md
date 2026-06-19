---
title: N-Body Simulation with SIMD
author: Matthias Kretz
date: 0000-03-08
layout: post
---

## Goal

Take your scalar N-body simulation from [the previous exercise][n-body] and accelerate it using [`std::simd`][ref-simd]. 
The key insight is that if you have $N$ bodies, you can pack all their positions, velocities, and accelerations into a 
single data-parallel type (which may or may not match a SIMD register) and compute interactions across lanes 
simultaneously.

This approach works best when $N$ is small enough to fit in one or two SIMD registers (typically 4–16 bodies on x86). 
For larger systems, you would chunk the data across multiple SIMD iterations.
However, this is complex enough, so we will limit ourselves to a single `simd::vec<T, N>` object per dimension.

> If you implemented "Infinite Space",
>
> you want to disable it or use a git branch to work on the simpler solution.
{:.block-warning}

## Setup

> Enable SIMD compilation
>
> In `nbody/CMakeLists.txt`, add a second build target that defines `USE_SIMD`. Also, let's enable `-Ofast` for these 
targets, if you haven't already done so. This is useful to get vector math functions from libmvec (glibc), since GCC 16 
does not ship an implementation of the math overloads yet.
>
> ```cmake
> add_runnable(nbody_scalar main.cpp)
> target_compile_options(nbody_scalar PRIVATE -Ofast)
>
> add_runnable(nbody_simd main.cpp)
> target_compile_options(nbody_simd PRIVATE -Ofast -DUSE_SIMD=1)
> ```
>
> Build both targets and compare performance later.
{:.block-task}

> fast-math 😱
>
> Many C++ experts shudder at the use of fast-math ([`-Ofast`][ref-ofast] implies [`-ffast-math`][ref-ffast-math]).
> And rightfully so. Be careful that you understand what you're doing to your application.
{:.block-warning}

## Create `simd_utils.h`

> Define SIMD helper utilities
>
> Create `nbody/simd_utils.h` containing the following building blocks:
>
> **Concept.** Define a concept that matches `std::simd::basic_vec` types:
>
> ```cpp
> #include <simd>
>
> namespace simd = std::simd;
>
> template <typename T>
>   concept simd_vec = std::same_as<T, simd::basic_vec<typename T::value_type, typename T::abi_type>>;
> ```
>
> **Math overloads.** Provide `sqrt` and `pow` overloads for SIMD vectors (they are in C++26, just not yet shipping):
>
> ```cpp
> template <simd_vec V>
>   constexpr V
>   sqrt(const V& x) { return V([&](int i) { return std::sqrt(x[i]); }); }
>
> template <simd_vec V>
>   constexpr V
>   pow(const V& x, typename V::value_type y) { return V([&](int i) { return std::pow(x[i], y); }); }
> ```
>
> **Rotate elements.** Implement a function that shifts all lanes by one position using `std::simd::permute` (the second 
argument defines an index mapping):
>
> ```cpp
> template <simd_vec V>
>   constexpr V
>   rotate_elements(const V& x)
>   { return simd::permute(x, [](int i, int size) { return (i + 1) % size; }); }
> ```
>
> **Delta overload.** (optional: if your code implemented infinite space) Add a `delta(V, V)` overload for SIMD vectors. For plain floating-point SIMD, this is just subtraction:
>
> ```cpp
> template <simd_vec V>
>   requires std::floating_point<typename V::value_type>
>   constexpr V
>   delta(V a, V b) { return a - b; }
> ```
>
> Include `"simd_utils.h"` where needed.
{:.block-task}

## Pack bodies into SIMD registers

> Rewrite `main` to initialize bodies as an array and conditionally pack them
>
> Store initial conditions in a `std::array<Body<float>, N>`:
>
> ```cpp
> auto bodies = init_bodies(std::array{
>   Body<float>{ {0, 0}, {}, {0, 0}, 2.f },
>   Body<float>{ {0, 1}, { 1, 0}, {0, 0}, 2.f },
>   Body<float>{ {0,-1}, {-1, 0}, {0, 0}, 2.f },
>   // ... more bodies
> });
> ```
>
> Write `init_bodies` as a template that returns either the original `std::array` (scalar mode) or a single `Body<V>` where `V = std::simd::vec<float, N>` (SIMD mode), controlled by `#ifdef USE_SIMD`. Use a generator constructor to build the `simd::vec`:
>
> ```cpp
> #ifdef USE_SIMD
>   using V = simd::vec<T, N>;
>   return Body<V> {
>     { V([&](int i) { return init[i].r.x; }), V([&](int i) { return init[i].r.y; }) },
>     { V([&](int i) { return init[i].v.x; }), V([&](int i) { return init[i].v.y; }) },
>     { V([&](int i) { return init[i].a.x; }), V([&](int i) { return init[i].a.y; }) },
>     V([&](int i) { return init[i].m; })
>   };
> #else
>   return init;
> #endif
> ```
>
> Each lane of the SIMD vector holds one body's coordinate. A `Body<simd::vec<float, 8>>` represents eight bodies 
packed into seven data-parallel objects.
{:.block-task}

> Visualize the memory layout of `Body<simd::vec<float, 4>>`
>
> - How does it depend on `-march` / CPU architecture?
> - Discuss caches, cache lines, and locality.
>
> (OK, the data is small and cache issues don't matter much here. But … can you extrapolate to different problems?)
{:.block-warning}

## Add SIMD overloads to simulation functions

> Duplicate each simulation function with a `simd_vec` overload
>
> Your existing functions operate on `std::span<Body<T>, N>`. Add parallel versions that operate on a single `Body<V>`:
>
> **Update positions:**
>
> ```cpp
> template <simd_vec V>
>   constexpr void
>   update_positions(const typename V::value_type dt, Body<V>& bodies)
>   {
>     // TODO
>   }
> ```
>
> **Update accelerations and velocities.** This is the core loop. Instead of iterating over pairs $(i, j)$, you iterate over rotated copies of the same SIMD register:
>
> ```cpp
> template <simd_vec V>
>   constexpr void
>   update_a_and_v(const typename V::value_type dt, Body<V>& bodies)
>   {
>     // TODO
>   }
> ```
>
> **Energy functions.** Use [`simd::reduce`][ref-reduce] to sum across lanes:
>
> ```cpp
> template <simd_vec V>
>   constexpr typename V::value_type
>   kinetic_energy(const Body<V>& bodies)
>   {
>     // TODO
>   }
> ```
>
> For potential energy, keeping the `j > i` condition isn't obvious. But there's an alternative: just compute more and 
make up for it in the end.
>
> ```cpp
> template <simd_vec V>
>   constexpr typename V::value_type
>   potential_energy(const Body<V>& bodies)
>   {
>     // TODO
>   }
> ```
>
> **Do timestep.** Simply dispatch to the SIMD versions:
>
> ```cpp
> template <simd_vec V>
>   constexpr void
>   do_timestep(typename V::value_type dt, Body<V>& bodies)
>   {
>     update_positions(dt, bodies);
>     update_a_and_v(dt, bodies);
>   }
> ```
{:.block-task}

## Performance measurement

> Benchmark scalar vs. SIMD
>
> Build both targets and measure wall-clock time using `hyperfine`. **Critical:** exclude any I/O from the timing 
window. Functions like `std::print`, `std::println`, and SFML drawing calls must be outside the measured loop.
>
> However, for my code, the removal of all I/O allowed GCC to see that the program has no effect and it eliminated everything as dead code. You can use `vir-inspect.sh` to check what the compiler is doing to your C++ code.
>
> - Use `hyperfine` and `perf stat` on the compiled binary.
> - Compare scalar and SIMD wall-clock times for the same number of timesteps and same body count.
> - Check SIMD instruction counts:
> ```bash
> schedtool -F -p 10 -a 1 -e \
> perf stat -e \
> fp_arith_inst_retired.scalar_single,\
> fp_arith_inst_retired.128b_packed_single,\
> fp_arith_inst_retired.256b_packed_single \
> ./nbody_simd
> ```
{:.block-task}

> Why this matters
>
> Including I/O in benchmarks produces misleading numbers. `std::println` and SFML rendering are orders of magnitude slower than physics computation and will completely dominate the timing. Always measure only the computational kernel.
{:.block-warning}

## Verify correctness

> Cross-check energy conservation
>
> Run both scalar and SIMD versions for the same initial conditions and step count. Compare final positions and total energy. They should agree within floating-point rounding tolerance.
{:.block-task}

## Further exploration

> Try different body counts
>
> Experiment with $N = 4, 8, 12, 16$ bodies. Observe how speedup changes with problem size. At what point does the overhead of packing/unpacking outweigh the SIMD benefit?
>
> Consider chunking: for $N >$ SIMD width, process groups of bodies in batches, accumulating partial forces before reducing.
{:.block-task}

> Visualize with SFML
>
> Adapt your SFML render loop to handle `Body<V>` by iterating over lanes:
>
> ```cpp
> const auto& [r, ..._, m] = bodies;
> for (int i = 0; i < V::size(); ++i)
>   {
>     sf::CircleShape point(m[i], 8);
>     point.setPosition({r.x[i] * scale + offset, r.y[i] * scale + offset});
>     window.draw(point);
>   }
> ```
>
> Remember: SFML drawing must be outside any timed benchmark loop.
{:.block-task}

> Keep inherent parallelism
>
> Fairly trivial, but in the snippet above, do you see the unnecessary serial execution for operations that are 
inherently data-parallel?
{:.block-tip}

---

## References

* [Data-parallel types (SIMD)][ref-simd] — cppreference (C++26)
* [Experimental SIMD library][ref-experimental-simd] — cppreference (`std::experimental::simd`, TS v2)
* [`std::experimental::reduce`][ref-experimental-reduce] — cppreference
* [`std::ranges::contiguous_range`][ref-contiguous-range] — cppreference
* [GCC `-Ofast`][ref-ofast] — GCC documentation
* [GCC `-ffast-math`][ref-ffast-math] — GCC documentation
* [Previous exercise: N-Body System Simulation][n-body]

[n-body]: 0000-00-00-n-body.html
[ref-simd]: https://en.cppreference.com/cpp/numeric/simd
[ref-experimental-simd]: https://en.cppreference.com/w/cpp/experimental/simd
[ref-experimental-reduce]: https://en.cppreference.com/w/cpp/experimental/simd/reduce
[ref-ofast]: https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html#index-Ofast
[ref-ffast-math]: https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html#index-ffast-math
[ref-contiguous-range]: https://en.cppreference.com/w/cpp/ranges/contiguous_range
