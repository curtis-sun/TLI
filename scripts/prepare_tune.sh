#! /usr/bin/env bash
# set -e

# echo "Compiling benchmark..."
# git submodule update --init --recursive 

# mkdir -p build
cd build
# cmake -DCMAKE_BUILD_TYPE=Release ..
# make -j 8 

function generate_uint64_200M() {
    echo "Generating operations for $1"
    ./generate ../data/$1 1000000 --negative-lookup-ratio 0.5
    ./generate ../data/$1 1000000 --insert-ratio 1 --bulkload-count 1000000
    ./generate ../data/$1 1000000 --insert-ratio 1 --insert-pattern delta --bulkload-count 1000000
    ./generate ../data/$1 1000000 --insert-ratio 1 --insert-pattern hotspot --hotspot-ratio 0.006 --bulkload-count 1000000
}

function generate_string_90M_multithread() {
    echo "Generating operations for $1 and $2 threads"
    ./generate ../data/$1 1000000 --thread $2 --bulkload-count 10000000
    ./generate ../data/$1 1000000 --insert-ratio 1 --thread $2 --bulkload-count 1000000
}

function generate_uint64_200M_multithread() {
    echo "Generating operations for $1 and $2 threads"
    ./generate ../data/$1 1000000 --negative-lookup-ratio 0.5 --thread $2
    ./generate ../data/$1 1000000 --insert-ratio 1 --thread $2 --bulkload-count 1000000
    ./generate ../data/$1 1000000 --insert-ratio 1 --thread $2 --insert-pattern delta --bulkload-count 1000000
    ./generate ../data/$1 1000000 --insert-ratio 1 --thread $2 --insert-pattern hotspot --hotspot-ratio 0.006 --bulkload-count 1000000
}

generate_uint64_200M fb_200M_uint64
generate_uint64_200M wiki_ts_200M_uint64
generate_uint64_200M osm_cellids_200M_uint64
generate_uint64_200M books_200M_uint64

generate_uint64_200M_multithread fb_200M_uint64 24
generate_uint64_200M_multithread wiki_ts_200M_uint64 24
generate_uint64_200M_multithread osm_cellids_200M_uint64 24
generate_uint64_200M_multithread books_200M_uint64 24

generate_string_90M_multithread url_90M_string 24