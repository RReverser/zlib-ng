set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR powerpc)
set(CMAKE_SYSTEM_VERSION 1)

set(CMAKE_C_COMPILER_TARGET "powerpc-linux-gnu")
set(CMAKE_CXX_COMPILER_TARGET "powerpc-linux-gnu")

set(CMAKE_CROSSCOMPILING TRUE)
set(CMAKE_CROSSCOMPILING_EMULATOR qemu-ppc -L /usr/${CMAKE_C_COMPILER_TARGET}/)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

find_program(C_COMPILER_FULL_PATH gcc-${CMAKE_C_COMPILER_TARGET} ${CMAKE_C_COMPILER_TARGET}-gcc)
if(NOT C_COMPILER_FULL_PATH)
    message(FATAL_ERROR "Cross-compiler ${CMAKE_C_COMPILER_TARGET}-gcc not found")
endif()
set(CMAKE_C_COMPILER ${C_COMPILER_FULL_PATH})

find_program(CXX_COMPILER_FULL_PATH g++-${CMAKE_C_COMPILER_TARGET} ${CMAKE_C_COMPILER_TARGET}-g++)
if(CXX_COMPILER_FULL_PATH)
    set(CMAKE_CXX_COMPILER ${CXX_COMPILER_FULL_PATH})
endif()
