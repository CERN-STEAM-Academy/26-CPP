---
---
Welcome to the **C++ course** of the CERN STEAM Academy 2026, taught by Matthias Kretz.

The course runs locally on the academy laptops. This page collects the course materials. For the general computing environment (accounts, WiFi, editors, lxplus, AFS, CERNBox) see the [STEAM Academy documentation](https://stac.docs.cern.ch/).

## Materials

- **Example exercise:** [exercise-example](https://github.com/CERN-STEAM-Academy/26-CPP/tree/main/exercise-example) — an n-body simulation and a `std::sin` lookup-table benchmark.

## Toolchain on the laptops

- **Compilers** (environment modules): `module load gcc/16.1.0` or `module load clang/22.1.5`.
- **Google Benchmark** and **SFML 3.1** are pulled in automatically by the example's CMake (`FetchContent`); the build prerequisites are preinstalled on the laptops.
- **Benchmarking:** `schedtool` / `chrt` / `taskset` for real-time scheduling without `sudo`, `perf` counters enabled, and a writable CPU governor — see the [environment docs](https://stac.docs.cern.ch/).

## Build and run the example

```sh
git clone https://github.com/CERN-STEAM-Academy/26-CPP.git
cd 26-CPP/exercise-example
module load gcc/16.1.0
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/sin_lut --benchmark_counters_tabular=true --benchmark_perf_counters=CYCLES,INSTRUCTIONS
```

## Schedule

The programme is on the [Indico timetable](https://indico.cern.ch/event/1697464/timetable).
