cmake_minimum_required(VERSION 3.14)
project(Exercises)

include(FetchContent)

# Fetch Google Benchmark from its repository (built with libpfm so that
# --benchmark_perf_counters works); declared before CMAKE_CXX_STANDARD is
# set so Benchmark builds with its own standard.
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_LIBPFM ON CACHE BOOL "" FORCE)
FetchContent_Declare(benchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG main
  GIT_SHALLOW ON)
FetchContent_MakeAvailable(benchmark)

find_package(Threads REQUIRED)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

function(add_benchmark name)
  if(ARGN)
    add_executable(${name} ${ARGN})
  else()
    add_executable(${name} ${name}.cpp)
  endif()
  target_link_libraries(${name} PRIVATE benchmark::benchmark Threads::Threads)
  add_custom_target(run_${name}
    COMMAND ${name} --benchmark_counters_tabular=true --benchmark_perf_counters=CYCLES,INSTRUCTIONS
    DEPENDS ${name}
    COMMENT "Execute ${name} benchmark"
    VERBATIM)
endfunction()

function(add_runnable name)
  if(ARGN)
    add_executable(${name} ${ARGN})
  else()
    add_executable(${name} ${name}.cpp)
  endif()
  add_custom_target(run_${name}
    COMMAND ${name}
    DEPENDS ${name}
    COMMENT "Execute ${name}"
    VERBATIM)
endfunction()

add_benchmark(sin_lut)

add_subdirectory(nbody)
