---
title: Exercise Setup
author: Dr. Matthias Kretz
date: 0000-01-02
layout: post
---

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
> cmake -S . -B build -G Ninja
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
> - `CMAKE_BUILD_TYPE` to `RelWithDebInfo`
> - `CMAKE_CXX_FLAGS_RELEASE` to `-O2 -DNDEBUG` and 
> - `CMAKE_CXX_FLAGS_RELWITHDEBINFO` to `-Og -g -DNDEBUG -fhardened`.
{:.block-task}

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

> Turn on C++26 features
>
> ```cmake
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
> ```
{:.block-task}
