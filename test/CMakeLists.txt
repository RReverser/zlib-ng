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

#set(BUILD_GMOCK OFF CACHE BOOL "Builds the googlemock subproject")
#set(INSTALL_GTEST OFF CACHE BOOL "Enable installation of googletest. (Projects embedding googletest may want to turn this OFF.)")

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

target_link_directories(gtest PRIVATE $ENV{LLVM_BUILD_DIR}/lib)
target_link_directories(gtest_main PRIVATE $ENV{LLVM_BUILD_DIR}/lib)
target_link_directories(gtest_zlib PRIVATE $ENV{LLVM_BUILD_DIR}/lib)

target_compile_options(gtest PRIVATE -stdlib=libc++)
target_compile_options(gtest_main PRIVATE -stdlib=libc++)
target_compile_options(gtest_zlib PRIVATE -stdlib=libc++)

target_link_options(gtest PRIVATE -stdlib=libc++ -lc++abi)
target_link_options(gtest_main PRIVATE -stdlib=libc++ -lc++abi)
target_link_options(gtest_zlib PRIVATE -stdlib=libc++ -lc++abi)

target_include_directories(gtest PRIVATE $ENV{LLVM_BUILD_DIR}/include $ENV{LLVM_BUILD_DIR}/include/c++/v1)
target_include_directories(gtest_main PRIVATE $ENV{LLVM_BUILD_DIR}/include $ENV{LLVM_BUILD_DIR}/include/c++/v1)
target_include_directories(gtest_zlib PRIVATE $ENV{LLVM_BUILD_DIR}/include $ENV{LLVM_BUILD_DIR}/include/c++/v1)

target_link_libraries(gtest_zlib zlibstatic gtest_main)

if(ZLIB_ENABLE_TESTS)
    add_test(NAME gtest_zlib
        COMMAND ${CMAKE_CROSSCOMPILING_EMULATOR} $<TARGET_FILE:gtest_zlib>)
endif()
