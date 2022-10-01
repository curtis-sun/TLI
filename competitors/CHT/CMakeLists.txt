cmake_minimum_required(VERSION 3.10)
project(cht)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3 -Wall -Wextra")

find_package(Threads REQUIRED)
set(THREADS_PREFER_PTHREAD_FLAG ON)

include("${CMAKE_SOURCE_DIR}/cmake_modules/googletest.cmake")

include_directories(
        ${GTEST_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}
)

file(GLOB INCLUDE_H "include/cht/*.h")
set(EXAMPLE_FILES example.cc)
set(BENCH_FILES bench.cc)
set(BENCH_END_TO_END_FILES bench_end_to_end.cc)
file(GLOB TEST_CC "test/*_test.cc")

add_executable(example ${INCLUDE_H} ${EXAMPLE_FILES})
add_executable(bench ${INCLUDE_H} ${BENCH_FILES})
add_executable(bench_end_to_end ${INCLUDE_H} ${BENCH_END_TO_END_FILES})

add_executable(tester ${TEST_CC})
target_link_libraries(tester gtest gtest_main Threads::Threads)
