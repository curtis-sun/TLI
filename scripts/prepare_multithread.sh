#! /usr/bin/env bash
# set -e

# echo "Compiling benchmark..."
# git submodule update --init --recursive 

# mkdir -p build
cd build
# cmake -DCMAKE_BUILD_TYPE=Release ..
# make -j 8 
function generate_uint64_200M() {
    echo "Generating operations for $1 and $2 threads"
    ./generate ../data/$1 29523809 --insert-ratio 0.322581 --negative-lookup-ratio 0.5 --thread $2
    ./generate ../data/$1 38181818 --insert-ratio 0.476190 --negative-lookup-ratio 0.5 --thread $2
    ./generate ../data/$1 86666666 --insert-ratio 0.769231 --negative-lookup-ratio 0.5 --thread $2
    ./generate ../data/$1 120000000 --insert-ratio 0.833333 --negative-lookup-ratio 0.5 --thread $2
    ./generate ../data/$1 186666666 --insert-ratio 0.892857 --negative-lookup-ratio 0.5 --thread $2
    ./generate ../data/$1 201818181 --insert-ratio 0.900901 --negative-lookup-ratio 0.5 --thread $2
    ./generate ../data/$1 207500000 --insert-ratio 0.903614 --negative-lookup-ratio 0.5 --thread $2
    
    ./generate ../data/$1 20000000 --insert-ratio 0.5 --negative-lookup-ratio 0.5 --bulkload-count 10000000 --thread $2
    ./generate ../data/$1 20000000 --insert-ratio 0.5 --negative-lookup-ratio 0.5 --insert-pattern delta --bulkload-count 10000000 --thread $2
    ./generate ../data/$1 20000000 --insert-ratio 0.5 --negative-lookup-ratio 0.5 --insert-pattern hotspot --hotspot-ratio 0.05 --bulkload-count 10000000 --thread $2
    ./generate ../data/$1 20000000 --insert-ratio 0.5 --negative-lookup-ratio 0.5 --insert-pattern hotspot --hotspot-ratio 0.050005 --bulkload-count 10000000 --thread $2
    ./generate ../data/$1 20000000 --insert-ratio 0.5 --negative-lookup-ratio 0.5 --insert-pattern hotspot --hotspot-ratio 0.05001 --bulkload-count 10000000 --thread $2
    ./generate ../data/$1 20000000 --insert-ratio 0.5 --negative-lookup-ratio 0.5 --insert-pattern hotspot --hotspot-ratio 0.050015 --bulkload-count 10000000 --thread $2 
    ./generate ../data/$1 20000000 --insert-ratio 0.5 --negative-lookup-ratio 0.5 --insert-pattern hotspot --hotspot-ratio 0.05002 --bulkload-count 10000000 --thread $2 
    ./generate ../data/$1 20000000 --insert-ratio 0.5 --negative-lookup-ratio 0.5 --insert-pattern hotspot --hotspot-ratio 0.050025 --bulkload-count 10000000 --thread $2 
    ./generate ../data/$1 20000000 --insert-ratio 0.5 --negative-lookup-ratio 0.5 --insert-pattern hotspot --hotspot-ratio 0.05003 --bulkload-count 10000000 --thread $2
}

function generate_uint64_200M_multithread() {
    echo "Generating operations for $1 and $2 threads"
    ./generate ../data/$1 100000000 --negative-lookup-ratio 0.5 --bulkload-count 105000000 --thread $2
    ./generate ../data/$1 100000000 --negative-lookup-ratio 0.5 --insert-ratio 0.05 --mix --bulkload-count 5000000 --thread $2
    ./generate ../data/$1 100000000 --negative-lookup-ratio 0.5 --insert-ratio 0.5 --mix --bulkload-count 5000000 --thread $2
    ./generate ../data/$1 100000000 --insert-ratio 1 --bulkload-count 5000000 --thread $2
    ./generate ../data/$1 100000000 --bulkload-count 5000000 --insert-ratio 0.05 --thread $2 --scan-ratio 0.95 --mix
}

function generate_string_90M() {
    echo "Generating operations for $1 with $2 threads"
    ./generate ../data/$1 60000000 --bulkload-count 63000000 --thread $2
    ./generate ../data/$1 60000000 --insert-ratio 0.05 --mix --bulkload-count 3000000 --thread $2
    ./generate ../data/$1 60000000 --insert-ratio 0.5 --mix --bulkload-count 3000000 --thread $2
    ./generate ../data/$1 60000000 --insert-ratio 1 --bulkload-count 3000000 --thread $2
}

generate_uint64_200M fb_200M_uint64 24
generate_uint64_200M wiki_ts_200M_uint64 24
generate_uint64_200M osm_cellids_200M_uint64 24
generate_uint64_200M books_200M_uint64 24

for THREAD in 2 4 8 16 24 32
do
    generate_string_90M url_90M_string $THREAD

    generate_uint64_200M_multithread fb_200M_uint64 $THREAD
    generate_uint64_200M_multithread wiki_ts_200M_uint64 $THREAD
    generate_uint64_200M_multithread osm_cellids_200M_uint64 $THREAD
    generate_uint64_200M_multithread books_200M_uint64 $THREAD
done
