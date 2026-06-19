---
title: Nearest Neighbor
author: Dr. Matthias Kretz
date: 0000-03-07
layout: post
---

> Generate random values
>
> Write a function producing a `std::vector` of random `float` values.
{:.block-task}

> TIP
>
> Boilerplate for random number generation
> ```c++
> std::random_device rd;
> std::mt19937 gen(rd());
> std::uniform_real_distribution<float> rnd0_10(0.f, 10.f);
> ```
{:.block-tip}

> Find nearest neighbor
>
> Given another random value, find the closest neighbor in the random `vector` and 
return its index in the `vector`.
{:.block-task}

> Vectorize
>
> Copy the code (or generalize via `template`) and work with 
`stdx::native_simd<float>` instead.
>
> 1. Use `simd(address, stdx::element_aligned)` to load from an array of `float`
> 2. Use `std::vector<native_simd<float>>`
{:.block-task}

> Which is better, if at all? When?
{:.block-warning}

> Benchmark
>
> Benchmark the three variants, over different sizes of the `vector`
{:.block-task}

> TIP
>
> One benchmark function, benchmarking different `vector` sizes
> ```c++
> void name(benchmark::State& state) {
>   const std::size_t n = state.range(0);
>   std::vector<float> values(n);
>   // ...
> }
>
> BENCHMARK(name)->Range(8, 8 << 20);
> ```
{:.block-tip}

> Extend to 3-D
>
> Go from 1-dim to three dimensional.
>
> 1. AoS
> 2. SoA
> 3. AoVS
> 4. kd-tree (nothing for today, at least)
{:.block-task}

> Benchmark the results
{:.block-task}

## Further Reading

[Data-Structure Vectorization][1]

[1]:https://mattkretz.github.io/2021/07/29/data-structure-vectorization.html
