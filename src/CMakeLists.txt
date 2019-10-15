cmake_minimum_required(VERSION 3.10)

set(EXE cracker)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O2 -Werror -Wall")

find_package(Threads REQUIRED)

add_executable(${EXE}
    cracker.cc
    )

target_link_libraries(${EXE}
    gcrypt
    ${CMAKE_THREAD_LIBS_INIT}
    )
