# 26-CPP — CERN STEAM Academy 2026, C++ course

Material for the C++ course at the CERN STEAM Academy 2026 (Matthias Kretz). The course runs **locally on the academy laptops**. For the general computing environment (accounts, WiFi, editors, lxplus, AFS, CERNBox) see the [STEAM Academy documentation](https://stac.docs.cern.ch/).

## Contents

The [`exercise-example/`](exercise-example/) folder holds a worked example:

- **`sin_lut`** — a [Google Benchmark](https://github.com/google/benchmark) comparing `std::sin` against a `constexpr`-generated lookup table with linear interpolation.
- **`nbody`** — a small gravitational n-body simulation visualised with [SFML](https://www.sfml-dev.org/).

Google Benchmark and SFML 3.1 are fetched and built automatically by CMake (`FetchContent`) — no system packages for them are needed.

## Quick start on the course laptops

Run these in a terminal on any academy laptop (`ngt-stac-laptop-0` … `ngt-stac-laptop-14`). No `sudo` is required.

```sh
# 1. Make GCC 16.1.0 available (CERN LCG build, exposed as an environment module)
module load gcc/16.1.0
g++ --version                 # should print: g++ (GCC) 16.1.0

# 2. Get the code
git clone https://github.com/CERN-STEAM-Academy/26-CPP.git
cd 26-CPP/exercise-example

# 3. Configure + build (Google Benchmark and SFML 3.1 are downloaded automatically)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j

# 4. Run the benchmark, with hardware performance counters
./build/sin_lut --benchmark_counters_tabular=true --benchmark_perf_counters=CYCLES,INSTRUCTIONS

# 5. Run the n-body simulation (opens a window — use the laptop's screen, not over SSH)
./build/nbody/nbody
```

### Making GCC 16 available

`module load gcc/16.1.0` is the recommended way; list what is installed with `module avail`. A few tips:

- The `module` command is set up at login. If you get `module: command not found` (e.g. in a non-login shell), enable it with `source /etc/profile.d/modules.sh` and try again.
- To use Clang instead: `module switch gcc/16.1.0 clang/22.1.5`.
- Without modules, you can source the compiler directly:

  ```sh
  source /cvmfs/sft.cern.ch/lcg/contrib/gcc/16.1.0/x86_64-el9/setup.sh
  g++ --version
  ```

`module load` (or the setup script) only affects the **current shell**, so run it again in each new terminal — or add the `module load gcc/16.1.0` line to your `~/.bashrc`.

## Requirements (building elsewhere)

On the course laptops everything below is already in place. To build off the laptops you need:

- A **C++26** compiler (GCC 16 / recent Clang) and **CMake ≥ 3.14**.
- Development packages so CMake can build SFML and Google Benchmark from source:
  - **AlmaLinux / RHEL:** `freetype-devel libX11-devel libXrandr-devel libXcursor-devel libXi-devel mesa-libGL-devel systemd-devel libpfm-devel`
  - **Debian / Ubuntu:** `libfreetype-dev libx11-dev libxrandr-dev libxcursor-dev libxi-dev libgl1-mesa-dev libudev-dev libpfm4-dev`

## Build

All build commands are run from the `exercise-example/` directory.

```sh
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

- The example builds against **SFML 3.1.0** (the original mixed the SFML 2/3 API). Changes: SFML is fetched via `FetchContent`; the event loop uses `std::optional` + `event->is<sf::Event::Closed>()`; `CircleShape::setPosition` takes a vector.
- Google Benchmark is fetched from its repository and built with **libpfm** so `--benchmark_perf_counters` works.
