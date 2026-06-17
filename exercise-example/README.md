# nbody / sin_lut — C++ benchmarking example

Setup, build and run instructions — including how to make GCC 16 available on the course laptops — are in the repository's [main README](../README.md).

This folder contains:

- **`sin_lut`** — a [Google Benchmark](https://github.com/google/benchmark) comparing `std::sin` against a `constexpr`-generated lookup table with linear interpolation.
- **`nbody`** — a small gravitational n-body simulation visualised with [SFML](https://www.sfml-dev.org/) 3.1.
