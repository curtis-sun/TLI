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

set(MKLROOT "/home/zhaoyan/intel/oneapi/mkl/2022.1.0")
set(TBBROOT "/home/zhaoyan/intel/oneapi/tbb/2021.6.0")

add_subdirectory(dtl)

if (${APPLE})
    include_directories(/usr/local/include/) # required by Mac OS to find boost
endif ()

include_directories(${MKLROOT}/include)
link_directories(${TBBROOT}/lib/intel64/gcc4.8)

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

set(MKL_LIBRARIES ${MKLROOT}/lib/intel64/libmkl_intel_lp64.a ${MKLROOT}/lib/intel64/libmkl_tbb_thread.a ${MKLROOT}/lib/intel64/libmkl_core.a)
target_link_libraries(benchmark
        PRIVATE -Wl,--start-group ${MKL_LIBRARIES} -Wl,--end-group
        PRIVATE dl
        -ltbb)
