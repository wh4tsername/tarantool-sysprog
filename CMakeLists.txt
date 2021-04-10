cmake_minimum_required(VERSION 3.17)
project(tarantool-sysprog C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")

set(CMAKE_C_STANDARD 11)

add_subdirectory(coro-sort)
add_subdirectory(shell)
