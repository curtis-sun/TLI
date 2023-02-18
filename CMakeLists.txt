cmake_minimum_required(VERSION 3.10)
project(WOSD)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffast-math -Wall -Wfatal-errors -march=native")

# Enable OpenMP if available
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(-fopenmp HAS_OPENMP)
if (HAS_OPENMP)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
endif()

set(CMAKE_CXX_STANDARD 17)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS chrono)

find_library(JemallocLib jemalloc)
find_library(TbbLib tbb)

set(MKL_INCLUDE_DIRECTORY "/opt/intel/mkl/include")
set(MKL_LINK_DIRECTORY "/opt/intel/mkl/lib/intel64")

add_subdirectory(dtl)

if (${APPLE})
    include_directories(/usr/local/include/) # required by Mac OS to find boost
endif ()

include_directories(${MKL_INCLUDE_DIRECTORY})
link_directories(${MKL_LINK_DIRECTORY})

set(SOURCE_FILES util.h)
file(GLOB_RECURSE RMI_SOURCES "competitors/rmi/*.cpp")
file(GLOB_RECURSE BENCH_SOURCES "benchmarks/*.cc")
file(GLOB_RECURSE SEARCH_SOURCES "searches/*.h" "searches/search.cpp")

if (${APPLE})
    # Certain headers (e.g., byteswap) not supported under Mac OS.
    list(REMOVE_ITEM BENCH_SOURCES "${CMAKE_SOURCE_DIR}/benchmarks/benchmark_wormhole.cc")
endif ()

add_executable(searches searches.cpp ${SOURCE_FILES} ${SEARCH_SOURCES})
add_executable(generate generate.cc ${SOURCE_FILES})
target_include_directories(generate
        PRIVATE "competitors/finedex/include")
target_link_libraries(generate Threads::Threads)

if (${LINUX})
    add_custom_target(
            libwh
            COMMAND make libwh.so
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/wormhole/
    )
endif ()

add_executable(benchmark benchmark.cc ${SOURCE_FILES} ${RMI_SOURCES} ${BENCH_SOURCES} ${SEARCH_SOURCES})

target_compile_definitions(benchmark PRIVATE NDEBUGGING)

target_include_directories(benchmark
        PRIVATE "competitors/CHT/include"
        PRIVATE "competitors/FST/include"
        PRIVATE "competitors/PGM-index/include"
        PRIVATE "competitors/rs/include"
        PRIVATE "competitors/stx-btree-0.9/include"
        PRIVATE "competitors/ts/include"
        PRIVATE ${Boost_INCLUDE_DIRS})

if (${LINUX})
    target_include_directories(benchmark PRIVATE "wormhole")
    target_link_libraries(benchmark PRIVATE ${CMAKE_SOURCE_DIR}/wormhole/libwh.so)
    add_dependencies(benchmark libwh)
endif ()

target_link_libraries(benchmark
        PRIVATE Threads::Threads dtl
        PRIVATE ${Boost_LIBRARIES})
        
target_link_libraries(benchmark
        PRIVATE ${TbbLib} 
        PRIVATE ${JemallocLib}
        PRIVATE ${CMAKE_THREAD_LIBS_INIT})

target_link_libraries(benchmark
        PRIVATE dl
        PRIVATE mkl_rt)
