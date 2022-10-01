cmake_minimum_required(VERSION 3.2)
project(xxx)
ADD_DEFINITIONS(-std=c++17)

include_directories("include")
include_directories("/opt/intel/oneapi/mkl/2021.1.1/include")

set(CMAKE_CXX_FLAGS "-g -msse -march=core-avx2")



add_executable(osm "osm.cpp" )
target_link_libraries(osm pthread)

