#!/usr/bin/env bash

mkdir -p build && cd build && cmake -DCMAKE_BUILD_TYPE=Release .. && make -j20 && cd ..

DATA_SETS="osm_cellids_200M_uint64 wiki_ts_200M_uint64 fb_200M_uint64 books_200M_uint64"

for DATA_SET in $DATA_SETS; do
  ./build/bench_end_to_end ../TLI/data/${DATA_SET} ../${DATA_SET}_equality_lookups_10M rs
  ./build/bench_end_to_end ../TLI/data/${DATA_SET} ../${DATA_SET}_equality_lookups_10M ts
done;