cmake_minimum_required(VERSION 3.10)
project(ts)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3 -Wall -Wextra")

find_package(Threads REQUIRED)
set(THREADS_PREFER_PTHREAD_FLAG ON)

file(GLOB INCLUDE_H "include/ts/*.h")
file(GLOB INCLUDE_H "include/rs/*.h")
set(EXAMPLE_FILES example.cc)
set(BENCH_END_TO_END_FILES bench_end_to_end.cc)

add_executable(example ${INCLUDE_H} ${EXAMPLE_FILES})
add_executable(bench_end_to_end ${INCLUDE_H} ${BENCH_END_TO_END_FILES})
