---
title: Exercise Setup
author: Dr. Matthias Kretz
date: 0000-01-02
layout: post
---

> Use GCC 16
>
> Since we're using some of the latest features of C++, we use the most recent (released) compiler.
>
> This should get you there:
> ```sh
> module load gcc/16
> ```
>
> The default compiler (GCC) on the system comes from the distribution's package manager.
> Likewise, the standard library (libstdc++) provided by the distribution matches that GCC version.
> Note that GCC 16 automatically builds against the libstdc++ headers it ships.
{:.block-task}

> If you need to install GCC 16
>
> If you want to build it quicker add `--disable-bootstrap` to the GCC 
> `configure` flags.
>
> ```sh
> cd $src
> git clone --depth 1 --single-branch --branch releases/gcc-16 \
>  git://gcc.gnu.org/git/gcc
>
> cd gcc
> ./contrib/download_prerequisites
> mkdir build
> cd build
> ../configure --prefix=$prefix/gcc16 --enable-languages=c++ --disable-multilib
> make -j $CMAKE_BUILD_PARALLEL_LEVEL
> make install
> export CC=$prefix/gcc16/bin/gcc
> export CXX=$prefix/gcc16/bin/g++
> ```
{:.block-tip}

> Set up git and cmake
>
> ```sh
> mkdir -p ~/src/exercises
> cd ~/src/exercises
> git init
> ```
>
> Open `CMakeLists.txt in your editor and add:
> ```cmake
> cmake_minimum_required(VERSION 3.30)
> project(Exercises)
> ```
>
> Next we'll set up a dependency to Google Benchmark:
> ```cmake
> find_package(benchmark 1.9 REQUIRED)
> ```
>
> In a terminal try
> ```sh
> cmake -B build -G Ninja
> ```
>
> This fails, since it can't find the required benchmark library.
> Instead of installing it, we now use CMake's `FetchContent` to get it automatically and contained in our project. (Note the change from `REQUIRED` to `QUIET`.)
>
> ```cmake
find_package(benchmark 1.9 QUIET)
if (NOT benchmark_FOUND)
  include(FetchContent)
  FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark
    GIT_TAG main)
  option(BENCHMARK_ENABLE_TESTING "" OFF)
  option(BENCHMARK_ENABLE_GTEST_TESTS "" OFF)
  option(BENCHMARK_ENABLE_LIBPFM "" ON) # for performance counters
  FetchContent_MakeAvailable(benchmark)
endif()
> ```
{:.block-task}

> Turn on C++26 features
>
> ```cmake
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
> ```
{:.block-task}

> Run the above cmake command again.
{:.block-task}

> Commit to git.
{:.block-task}

> Change the cmake configuration.
>
> Use `ccmake` to see all options:
> ```sh
> ccmake build
> ```
> The key `t` toggles "advanced mode". If you turn it on, you will see the compiler flags used for the different build modes.
> We only care about `CXX_FLAGS` variables.
>
> For now, change
> - `CMAKE_CXX_FLAGS_RELEASE` to `-O2 -DNDEBUG` and 
> - `CMAKE_CXX_FLAGS_RELWITHDEBINFO` to `-Og -g -DNDEBUG -fhardened`.
{:.block-task}

> Why `-O2` rather than `-O3`
>
> I am very unhappy about the cmake default. Consider this quote from a GCC developer:
> "`-O2` means 'optimise well'.
> `-O3` means 'throw everything at it, it may be slower as well, but almost certainlly is way too big'"
>
> Defaulting to `-O3` is premature optimization, IMHO. Measure, and then decide whether `-O3` actually helps. Use it 
only if it measurably improves your program.
{:.block-tip}

> Compiler flags
>
> Feel free to experiment with different compiler flags.
{:.block-tip}

> Verify with a dummy executable.
>
> Add a `test.cpp` with:
> ```c++
#include <benchmark/benchmark.h>
>
int main() { }
> ```
>
> Register with cmake in `CMakeLists.txt`:
> ```cmake
add_executable(test test.cpp)
target_link_libraries(test PRIVATE benchmark::benchmark)
> ```
>
> Build either with
> ```sh
> cmake --build build
> ```
> or
> ```sh
> cd build
> ninja
> ```
{:.block-task}

> Optional: Improve [LSP][3] support
>
> If you use `clangd` as an [LSP][3], add a `.clangd` file that points to GCC 16's libstdc++:
> ```
> CompileFlags:
>  Add:
>    - --gcc-toolchain=/path/to/gcc-16
> ```
> And add `set(CMAKE_EXPORT_COMPILE_COMMANDS ON)` to `CMakeLists.txt`.
>
> That way CMake will generate `compile_commands.json` file, and `clangd` knows exactly how to compile your code as you go.
>
> (I use the `ale` plugin for `vim`.)
{:.block-tip}

[3]: https://en.wikipedia.org/wiki/Language_Server_Protocol
