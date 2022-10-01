cmake_minimum_required(VERSION 2.6)
project(FST)

message(STATUS "Configuring..." ${CMAKE_PROJECT_NAME})


# ---------------------------------------------------------------------------
# Include
# ---------------------------------------------------------------------------


if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif ()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g -Wall -mpopcnt -pthread -std=c++17")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3 -Wall -Werror -mpopcnt -pthread -std=c++17")


# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------

find_package(Threads REQUIRED)
set(THREADS_PREFER_PTHREAD_FLAG ON)
include("${CMAKE_SOURCE_DIR}/vendor/gtest.cmake")

# ---------------------------------------------------------------------------
# Code Coverage
# ---------------------------------------------------------------------------

option(COVERALLS "Generate coveralls data" OFF)

if (COVERALLS)
    include("${CMAKE_CURRENT_SOURCE_DIR}/CodeCoverage.cmake")
    append_coverage_compiler_flags()
    set(COVERAGE_EXCLUDES 'ARF/*' 'bench/*' 'test/*' '/usr/*' '/lib/*')
    setup_target_for_coverage(
            NAME coverage
            EXECUTABLE make test
    )
else ()
    add_definitions(-DNDEBUG)
endif ()

enable_testing()

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

# ---------------------------------------------------------------------------
# Clang Format and Tidy
# ---------------------------------------------------------------------------

find_program(CLANG_FORMAT_EXECUTABLE NAMES clang-format)

# Gather lintable files - exclude {popcount, rank, select, bitvector, config, hash, label_vector}.h
file(GLOB ALL_SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/include/fst*"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/louds_*"
        )

message(${ALL_SOURCE_FILES})

add_custom_target(
        clang-format
        COMMAND /usr/bin/clang-format
        -style=Google
        -CompactNamespaces: true
        -i
        ${ALL_SOURCE_FILES}
)

add_custom_target(
        clang-tidy
        COMMAND clang-tidy
        ${ALL_SOURCE_FILES}
        -config=''
        -format-style=google
        --
        ${INCLUDE_DIRECTORIES}
)


# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------

include("${CMAKE_SOURCE_DIR}/test/local.cmake")

# ---------------------------------------------------------------------------
# Benchmarking
# ---------------------------------------------------------------------------
#add_subdirectory(bench)

#include_directories("${CMAKE_CURRENT_SOURCE_DIR}/ARF/include")
#add_subdirectory(ARF)
