#! /usr/bin/env bash
# set -e

# echo "Compiling benchmark..."
# git submodule update --init --recursive 

# mkdir -p build
cd build
# cmake -DCMAKE_BUILD_TYPE=Release ..
# make -j 8 

function generate_lookups_200M() {
    echo "Generating lookups for $1"
    ./generate ../data/$1 1000000 --negative-lookup-ratio 0.5
    ./generate ../data/$1 1000000 --insert-ratio 1
    ./generate ../data/$1 1000000 --insert-ratio 1 --insert-mode delta
    ./generate ../data/$1 1000000 --insert-ratio 1 --insert-mode hotspot --hotspot-ratio 0.006
}

function generate_strings_90M() {
    echo "Generating lookups for $1 and $2 threads"
    ./generate ../data/$1 1000000 --thread $2
    ./generate ../data/$1 1000000 --insert-ratio 1 --thread $2
}

function generate_lookups_200M_multithread() {
    echo "Generating lookups for $1 and $2 threads"
    ./generate ../data/$1 1000000 --thread $2
    ./generate ../data/$1 1000000 --insert-ratio 1 --thread $2
}

echo "Generating queries..."
generate_lookups_200M fb_200M_uint64
generate_lookups_200M wiki_ts_200M_uint64
generate_lookups_200M osm_cellids_200M_uint64
generate_lookups_200M books_200M_uint64

generate_lookups_200M_multithread fb_200M_uint64 24
generate_lookups_200M_multithread wiki_ts_200M_uint64 24
generate_lookups_200M_multithread osm_cellids_200M_uint64 24
generate_lookups_200M_multithread books_200M_uint64 24

generate_strings_90M url_90M_string 24