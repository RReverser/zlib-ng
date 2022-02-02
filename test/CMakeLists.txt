cmake_minimum_required(VERSION 3.12)

include(FetchContent)

enable_language(CXX)

# Google test requires at least C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL
    "Use shared (DLL) run-time lib even when Google Test is built as static lib." FORCE)
set(gtest_disable_pthreads ON CACHE BOOL
    "Disable uses of pthreads in gtest." FORCE)

# Fetch Google test source code from official repository
FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG main)

FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
  FetchContent_Populate(googletest)
  add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

file(GLOB TEST_SRCS
    LIST_DIRECTORIES false
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/test_*.cc)

add_executable(gtest_zlib ${TEST_SRCS})

target_include_directories(gtest_zlib PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR})

#if(WITH_SANITIZER STREQUAL "Memory")
    # C++ targets must be linked against MSAN-linked C++ libraries
    macro(configure_cxx_msan target)
        target_link_directories(${target} PRIVATE $ENV{LLVM_BUILD_DIR}/lib)
        target_link_libraries(${target} PRIVATE c++abi)
        target_include_directories(${target} PRIVATE
            $ENV{LLVM_BUILD_DIR}/include
            $ENV{LLVM_BUILD_DIR}/include/c++/v1)
        target_compile_options(${target} PRIVATE -stdlib=libc++)
        target_link_options(${target} PRIVATE -stdlib=libc++)
    endmacro()

    configure_cxx_msan(gtest)
    configure_cxx_msan(gtest_main)
    configure_cxx_msan(gtest_zlib)
#endif()

target_link_libraries(gtest_zlib zlibstatic gtest_main)

if(ZLIB_ENABLE_TESTS)
    add_test(NAME gtest_zlib
        COMMAND ${CMAKE_CROSSCOMPILING_EMULATOR} $<TARGET_FILE:gtest_zlib>)
endif()
