cmake_minimum_required(VERSION 3.2)

add_executable(rmi_segmentation rmi_segmentation.cpp)
add_executable(rmi_errors rmi_errors.cpp)
add_executable(rmi_intervals rmi_intervals.cpp)
add_executable(rmi_lookup rmi_lookup.cpp)
add_executable(rmi_build rmi_build.cpp)
add_executable(rmi_guideline rmi_guideline.cpp)

set(SOSD_PATH "${PROJECT_SOURCE_DIR}/third_party/RMI/include/rmi_ref")
add_executable(index_comparison
    index_comparison.cpp
    ${SOSD_PATH}/books_200M_uint64_0.cpp
    ${SOSD_PATH}/books_200M_uint64_1.cpp
    ${SOSD_PATH}/books_200M_uint64_2.cpp
    ${SOSD_PATH}/books_200M_uint64_3.cpp
    ${SOSD_PATH}/books_200M_uint64_4.cpp
    ${SOSD_PATH}/books_200M_uint64_5.cpp
    ${SOSD_PATH}/books_200M_uint64_6.cpp
    ${SOSD_PATH}/books_200M_uint64_7.cpp
    ${SOSD_PATH}/books_200M_uint64_8.cpp
    ${SOSD_PATH}/books_200M_uint64_9.cpp
    ${SOSD_PATH}/fb_200M_uint64_0.cpp
    ${SOSD_PATH}/fb_200M_uint64_1.cpp
    ${SOSD_PATH}/fb_200M_uint64_2.cpp
    ${SOSD_PATH}/fb_200M_uint64_3.cpp
    ${SOSD_PATH}/fb_200M_uint64_4.cpp
    ${SOSD_PATH}/fb_200M_uint64_5.cpp
    ${SOSD_PATH}/fb_200M_uint64_6.cpp
    ${SOSD_PATH}/fb_200M_uint64_7.cpp
    ${SOSD_PATH}/fb_200M_uint64_8.cpp
    ${SOSD_PATH}/fb_200M_uint64_9.cpp
    ${SOSD_PATH}/osm_cellids_200M_uint64_0.cpp
    ${SOSD_PATH}/osm_cellids_200M_uint64_1.cpp
    ${SOSD_PATH}/osm_cellids_200M_uint64_2.cpp
    ${SOSD_PATH}/osm_cellids_200M_uint64_3.cpp
    ${SOSD_PATH}/osm_cellids_200M_uint64_4.cpp
    ${SOSD_PATH}/osm_cellids_200M_uint64_5.cpp
    ${SOSD_PATH}/osm_cellids_200M_uint64_6.cpp
    ${SOSD_PATH}/osm_cellids_200M_uint64_7.cpp
    ${SOSD_PATH}/osm_cellids_200M_uint64_8.cpp
    ${SOSD_PATH}/osm_cellids_200M_uint64_9.cpp
    ${SOSD_PATH}/wiki_ts_200M_uint64_0.cpp
    ${SOSD_PATH}/wiki_ts_200M_uint64_1.cpp
    ${SOSD_PATH}/wiki_ts_200M_uint64_2.cpp
    ${SOSD_PATH}/wiki_ts_200M_uint64_3.cpp
    ${SOSD_PATH}/wiki_ts_200M_uint64_4.cpp
    ${SOSD_PATH}/wiki_ts_200M_uint64_5.cpp
    ${SOSD_PATH}/wiki_ts_200M_uint64_6.cpp
    ${SOSD_PATH}/wiki_ts_200M_uint64_7.cpp
    ${SOSD_PATH}/wiki_ts_200M_uint64_8.cpp
    ${SOSD_PATH}/wiki_ts_200M_uint64_9.cpp
)
