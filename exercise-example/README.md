# nbody / sin_lut — C++ benchmarking example

Example for the CERN STEAM Academy 2026 C++ course (Matthias Kretz). It contains:

- **`sin_lut`** — a [Google Benchmark](https://github.com/google/benchmark) comparing `std::sin` against a `constexpr`-generated lookup table with linear interpolation.
- **`nbody`** — a small gravitational n-body simulation visualised with [SFML](https://www.sfml-dev.org/).

## Requirements

- A **C++26** compiler. On the course laptops: `module load gcc/16.1.0`.
- **CMake ≥ 3.14**.
- Google Benchmark and **SFML 3.1.0** are fetched and built automatically by CMake (`FetchContent`) — no system packages for them are needed.
- Building SFML and Google Benchmark from source needs these development packages (already installed on the course laptops):
  - **AlmaLinux / RHEL:** `freetype-devel libX11-devel libXrandr-devel libXcursor-devel libXi-devel mesa-libGL-devel systemd-devel libpfm-devel`
  - **Debian / Ubuntu:** `libfreetype-dev libx11-dev libxrandr-dev libxcursor-dev libxi-dev libgl1-mesa-dev libudev-dev libpfm4-dev`

## Build

```sh
module load gcc/16.1.0          # course laptops only
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

There is also a convenience `Makefile` (wraps the above into a per-compiler build directory) — just run `make`.

## Run

### sin_lut benchmark

```sh
./build/sin_lut

# with hardware performance counters (needs libpfm + kernel.perf_event_paranoid <= 1):
./build/sin_lut --benchmark_counters_tabular=true --benchmark_perf_counters=CYCLES,INSTRUCTIONS

# or the convenience target:
cmake --build build --target run_sin_lut
```

For stable measurements, pin a core and run at real-time priority (no `sudo` needed on the laptops) and use the `performance` CPU governor:

```sh
schedtool -F -p 10 -a 2 -e ./build/sin_lut
```

### nbody simulation (graphical)

```sh
./build/nbody/nbody     # opens an SFML window — run at the machine's display, not over SSH
```

## Notes

- Adapted to build against **SFML 3.1.0** (the original mixed SFML 2/3 API). Changes: SFML is fetched via `FetchContent`; the event loop uses `std::optional` + `event->is<sf::Event::Closed>()`; `CircleShape::setPosition` takes a vector.
- Google Benchmark is built with **libpfm** so `--benchmark_perf_counters` works.
