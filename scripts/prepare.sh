#! /usr/bin/env bash
# set -e

echo "Compiling benchmark..."
git submodule update --init --recursive 

mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j 8 

function generate_lookups_200M() {
    echo "Generating lookups for $1"
    ./generate ../data/$1 29523809 --insert-ratio 0.322581 --negative-lookup-ratio 0.5
    ./generate ../data/$1 38181818 --insert-ratio 0.476190 --negative-lookup-ratio 0.5
    ./generate ../data/$1 86666666 --insert-ratio 0.769231 --negative-lookup-ratio 0.5
    ./generate ../data/$1 120000000 --insert-ratio 0.833333 --negative-lookup-ratio 0.5
    ./generate ../data/$1 186666666 --insert-ratio 0.892857 --negative-lookup-ratio 0.5
    ./generate ../data/$1 201818181 --insert-ratio 0.900901 --negative-lookup-ratio 0.5
    ./generate ../data/$1 207500000 --insert-ratio 0.903614 --negative-lookup-ratio 0.5
    
    ./generate ../data/$1 20000000 --negative-lookup-ratio 0.5
    ./generate ../data/$1 20000000 --scan-ratio 1

    ./generate ../data/$1 100000000 --negative-lookup-ratio 0.5 --scan-ratio 0.05 --mix --bulkload-count 105000000
    ./generate ../data/$1 100000000 --negative-lookup-ratio 0.5 --insert-ratio 0.05 --scan-ratio 0.0475 --mix --bulkload-count 5000000
    ./generate ../data/$1 100000000 --negative-lookup-ratio 0.5 --insert-ratio 0.5 --scan-ratio 0.025 --mix --bulkload-count 5000000
    ./generate ../data/$1 100000000 --negative-lookup-ratio 0.5 --insert-ratio 0.8 --scan-ratio 0.01 --mix --bulkload-count 5000000
    ./generate ../data/$1 100000000 --insert-ratio 1 --bulkload-count 5000000
    
    ./generate ../data/$1 20000000 --insert-ratio 0.5 --negative-lookup-ratio 0.5 --bulkload-count 10000000
    ./generate ../data/$1 20000000 --insert-ratio 0.5 --negative-lookup-ratio 0.5 --insert-pattern delta --bulkload-count 10000000
    ./generate ../data/$1 20000000 --insert-ratio 0.5 --negative-lookup-ratio 0.5 --insert-pattern hotspot --hotspot-ratio 0.05 --bulkload-count 10000000
    ./generate ../data/$1 20000000 --insert-ratio 0.5 --negative-lookup-ratio 0.5 --insert-pattern hotspot --hotspot-ratio 0.1 --bulkload-count 10000000
    ./generate ../data/$1 20000000 --insert-ratio 0.5 --negative-lookup-ratio 0.5 --insert-pattern hotspot --hotspot-ratio 0.15 --bulkload-count 10000000
    ./generate ../data/$1 20000000 --insert-ratio 0.5 --negative-lookup-ratio 0.5 --insert-pattern hotspot --hotspot-ratio 0.2 --bulkload-count 10000000
    ./generate ../data/$1 20000000 --insert-ratio 0.5 --negative-lookup-ratio 0.5 --insert-pattern hotspot --hotspot-ratio 0.25 --bulkload-count 10000000
    ./generate ../data/$1 20000000 --insert-ratio 0.5 --negative-lookup-ratio 0.5 --insert-pattern hotspot --hotspot-ratio 0.3 --bulkload-count 10000000
}

function generate_strings_90M() {
    echo "Generating lookups for $1"
    ./generate ../data/$1 60000000 --bulkload-count 63000000
    ./generate ../data/$1 60000000 --insert-ratio 0.05 --mix --bulkload-count 3000000
    ./generate ../data/$1 60000000 --insert-ratio 0.5 --mix --bulkload-count 3000000
    ./generate ../data/$1 60000000 --insert-ratio 1 --bulkload-count 3000000
}

echo "Generating queries..."
generate_lookups_200M fb_200M_uint64
generate_lookups_200M wiki_ts_200M_uint64
generate_lookups_200M osm_cellids_200M_uint64
generate_lookups_200M books_200M_uint64

generate_strings_90M url_90M_string
