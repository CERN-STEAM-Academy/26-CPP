// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2016–2026 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
//                       Matthias Kretz <m.kretz@gsi.de>

#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <benchmark/benchmark.h>

///////////////////////////////////////////////////////////////////////////////
// element_count<T>
template <class T> struct element_count : std::integral_constant<std::size_t, 1> {};

template <typename T>
  concept has_ic_size
    = requires { T::size.value; } and T::size.value == T::size() and T::size() == T::size;

template <typename T>
  concept has_subscript_operator
    = requires(const T &x) { sizeof(x[0]); };

template <has_ic_size T>
  struct element_count<T> : decltype(T::size)
  {};

template <has_subscript_operator T>
  requires (not has_ic_size<T>)
  struct element_count<T>
    : std::integral_constant<std::size_t,
                             sizeof(T) / sizeof(std::declval<const T &>()[0])>
  {};

///////////////////////////////////////////////////////////////////////////////
// add_*_counters
static void
add_flop_counters(benchmark::State &state, auto flop_per_iteration)
{
  state.counters["FLOP"] = {static_cast<double>(flop_per_iteration),
                            benchmark::Counter::kIsIterationInvariantRate};
  if (state.counters.contains("CYCLES"))
    state.counters["FLOP/cycle"] = {flop_per_iteration / state.counters["CYCLES"],
                                    benchmark::Counter::kIsIterationInvariant};
}

static void
add_IPC_counters(benchmark::State &state)
{
  if (state.counters.contains("CYCLES") && state.counters.contains("INSTRUCTIONS"))
    state.counters["IPC"] = {state.counters["INSTRUCTIONS"] / state.counters["CYCLES"]};
}

template <typename T = float>
  static void
  add_throughput_counters(benchmark::State& state)
  {
    if constexpr (std::is_same_v<T, void>)
      {
        const double values_per_iteration = state.range(0);
        state.counters["throughput / (values per s)"] = {values_per_iteration,
                                                   benchmark::Counter::kIsIterationInvariantRate,
                                                   benchmark::Counter::kIs1024};

        if (state.counters.contains("CYCLES"))
          {
            state.counters["throughput / (values per cycle)"] = {
              values_per_iteration / state.counters["CYCLES"],
              benchmark::Counter::kIsIterationInvariant
            };
          }
      }
    else
      {
        const double bytes_per_iteration = state.range(0) * sizeof(T);
        state.counters["throughput / (Byte/s)"] = {double(bytes_per_iteration),
                                                   benchmark::Counter::kIsIterationInvariantRate,
                                                   benchmark::Counter::kIs1024};

        if (state.counters.contains("CYCLES"))
          {
            state.counters["throughput / (Bytes per cycle)"] = {
              double(bytes_per_iteration) / state.counters["CYCLES"],
              benchmark::Counter::kIsIterationInvariant
            };
          }
      }

    if (state.counters.contains("INSTRUCTIONS"))
      {
        state.counters["asm efficiency / (instructions per value)"] = {
          double(state.range(0)) / state.counters["INSTRUCTIONS"],
          benchmark::Counter::kIsIterationInvariant | benchmark::Counter::kInvert
        };
      }
  }

BENCHMARK_MAIN();
#endif // BENCHMARK_H
