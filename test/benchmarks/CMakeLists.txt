cmake_minimum_required(VERSION 3.12)

include(CheckCCompilerFlag)
include(FeatureSummary)
include(FetchContent)

enable_language(CXX)

# Fetch google benchmark source code from official repository
set(BENCHMARK_ENABLE_TESTING OFF)
FetchContent_Declare(benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git)
FetchContent_MakeAvailable(benchmark)
FetchContent_GetProperties(benchmark)

if(NOT benchmark_POPULATED)
    FetchContent_Populate(benchmark)
endif()

add_executable(zlib_benchmark
    benchmark_adler32.cc
    benchmark_compare258.cc
    benchmark_crc32.cc
    benchmark_main.cc)

target_include_directories(zlib_benchmark PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${benchmark_SOURCE_DIR}/benchmark/include)

target_link_libraries(zlib_benchmark zlibstatic benchmark)
if(WIN32)
    target_link_libraries(zlib_benchmark shlwapi)
endif()