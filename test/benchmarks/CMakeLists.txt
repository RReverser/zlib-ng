cmake_minimum_required(VERSION 3.12)

include(FetchContent)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
enable_language(CXX)

# Search for Google benchmark package
find_package(benchmark QUIET)
if(NOT benchmark_FOUND)
    # Fetch google benchmark source code from official repository
    set(BENCHMARK_ENABLE_TESTING OFF)
    FetchContent_Declare(benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git)
    FetchContent_MakeAvailable(benchmark)
    FetchContent_GetProperties(benchmark)

    if(NOT benchmark_POPULATED)
        FetchContent_Populate(benchmark)
    endif()
endif()

add_executable(benchmark_zlib
    benchmark_adler32.cc
    benchmark_compare256.cc
    benchmark_crc32.cc
    benchmark_main.cc
    benchmark_slidehash.cc
    )

target_include_directories(benchmark_zlib PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${benchmark_SOURCE_DIR}/benchmark/include)

target_link_libraries(benchmark_zlib zlibstatic benchmark::benchmark)
if(WIN32)
    target_link_libraries(benchmark_zlib shlwapi)
endif()

if(ZLIB_ENABLE_TESTS)
    add_test(NAME benchmark_zlib
        COMMAND ${CMAKE_CROSSCOMPILING_EMULATOR} $<TARGET_FILE:benchmark_zlib>)
endif()

if(WITH_BENCHMARK_APPS)
    option(BUILD_ALT_BENCH "Link against alternative zlib implementation" OFF)

    # Search for libpng package
    find_package(PNG QUIET)

    if(NOT PNG_FOUND)
        if(NOT ZLIB_COMPAT)
            message(FATAL_ERROR "PNG library requires zlib compatibility")
        endif()

        set(ZLIB_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR})
        set(ZLIB_LIBRARY ZLIB::ZLIB)

        add_library(ZLIB::ZLIB ALIAS zlib)
        set(SKIP_INSTALL_ALL ON)

        FetchContent_Declare(png
            GIT_REPOSITORY https://github.com/glennrp/libpng.git)
        FetchContent_MakeAvailable(png)
        FetchContent_GetProperties(png)
        if(NOT png_POPULATED)
            FetchContent_Populate(png)
        endif()

        set(PNG_INCLUDE_DIR ${png_SOURCE_DIR} ${png_BINARY_DIR})
        set(PNG_LIBRARIES png_static)

        target_include_directories(png_static PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
    endif()

    set(BENCH_APP_SRCS
        benchmark_png_encode.cc
        benchmark_png_decode.cc
        benchmark_main.cc
    )

    add_executable(benchmark_zlib_apps ${BENCH_APP_SRCS})
    target_include_directories(benchmark_zlib_apps PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_BINARY_DIR}
        ${PNG_INCLUDE_DIR}
        ${benchmark_SOURCE_DIR}/benchmark/include)

    # We need the static png library if we're statically linking to zlib,
    # otherwise it will resolve these things in the system provided dynamic
    # libraries (likely linked to stock zlib)
    target_link_libraries(benchmark_zlib_apps ${PNG_LIBRARIES} zlib benchmark::benchmark)

    if(BUILD_ALT_BENCH)
        set(ZLIB_ALT_LIB "libz.a" CACHE FILEPATH "Optional alternative zlib implementation (defaults to stock zlib)")
        add_executable(benchmark_zlib_apps_alt ${BENCH_APP_SRCS})
        target_link_libraries(benchmark_zlib_apps_alt libpng.a ${ZLIB_ALT_LIB} benchmark::benchmark)
        target_compile_definitions(benchmark_zlib_apps_alt PRIVATE BUILD_ALT=1)
        target_include_directories(benchmark_zlib_apps_alt PRIVATE
            ${CMAKE_SOURCE_DIR}
            ${CMAKE_BINARY_DIR}
            ${PNG_INCLUDE_DIR}
            ${benchmark_SOURCE_DIR}/benchmark/include)
    endif()
endif()
