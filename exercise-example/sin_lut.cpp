// SPDX-License-Identifier: BSD-3-Clause
// ---------------------------------------------------------------------------
// Benchmark: std::sin vs. constexpr-generated Lookup Table with
//            linear interpolation (std::lerp)
// ---------------------------------------------------------------------------
// Pedagogical goal:
//   Students discover that table-lookup approaches can dramatically beat
//   hardware transcendental instructions — but only when the table fits in
//   cache.  Accuracy vs. size trade-offs are explored by varying LUT entry
//   count.
// ---------------------------------------------------------------------------

#include <array>
#include <cmath>
#include <concepts>
#include <numbers>
#include <cassert>
#include <print>
#include <random>
#include <string_view>
#include <vector>

#include "benchmark.h"

template <std::floating_point T, std::size_t N = 256 << sizeof(T)>
  constexpr std::array<T, N + 1>
  lut_sin = [] consteval {
    std::array<T, N + 1> a;
    for (unsigned i = 0; i < N; ++i)
      a[i] = std::sin(2 * std::numbers::pi_v<T> * i / N);
    a[N] = a[0];
    return a;
  }();

// precondition: 0 <= x <= 2pi
template <std::floating_point T>
  [[nodiscard]] constexpr T
  my_sin(T x)
  {
    constexpr std::size_t N = lut_sin<T>.size() - 1;
    const T pos = N * x / (2 * std::numbers::pi_v<T>);
    const std::size_t idx = static_cast<std::size_t>(std::floor(pos));
    return std::lerp(lut_sin<T>[idx], lut_sin<T>[idx + 1], pos - std::floor(pos));
  }

// ============================================================================
// 4. Accuracy verification (runs once during benchmark init)
// ============================================================================

template <std::floating_point T>
  constexpr bool
  print_accuracy(std::string_view label)
  {
    std::philox4x32 rng(42);
    std::uniform_real_distribution<T> dist(T(0), T(2) * std::numbers::pi_v<T>);

    T max_err = T(0);
    constexpr int samples = 100'000;

    for (int i = 0; i < samples; ++i) {
					const T angle = dist(rng);
					const T err   = std::abs(my_sin(angle) - std::sin(angle));
					if (err > max_err) max_err = err;
				      }

    std::println("[accuracy] {:<10} max |error| = {:.2e}  ({} entries)",
		 label, max_err, lut_sin<T>.size());
    return max_err < T(1.e-7);
  }

const bool float_ok = print_accuracy<float>("float");
const bool double_ok = print_accuracy<double>("double");

// ============================================================================
// 5. Benchmarks
// ============================================================================

template <std::floating_point T, auto fun>
static void BM_sin(benchmark::State &state)
{
  std::vector<T> angles(1024 * 1024);

  std::philox4x32 rng(123);
  std::uniform_real_distribution<T> dist(T(0), T(2) * std::numbers::pi_v<T>);
  std::generate(angles.begin(), angles.end(), [&]() { return dist(rng); });

  for (auto _ : state) {
    T sum = 0.f;
    for (std::size_t i = 0; i < angles.size(); ++i)
      sum += fun(angles[i]);
    benchmark::DoNotOptimize(sum);
  }

  state.SetItemsProcessed(state.iterations() * angles.size());
}

BENCHMARK(BM_sin<float, [](float x) { return std::sin(x); }>);
BENCHMARK(BM_sin<float, [](float x) { return my_sin(x); }>);
BENCHMARK(BM_sin<double, [](double x) { return std::sin(x); }>);
BENCHMARK(BM_sin<double, [](double x) { return my_sin(x); }>);
