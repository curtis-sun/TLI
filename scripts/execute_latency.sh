#! /usr/bin/env bash

echo "Executing benchmark and saving results..."

BENCHMARK=build/benchmark
if [ ! -f $BENCHMARK ]; then
    echo "benchmark binary does not exist"
    exit
fi

function execute_uint64_200M() {
    echo "Executing operations for dataset $1 and index $2"
    $BENCHMARK ./data/$1 ./data/$1_ops_20M_0.000000rq_0.500000nl_0.000000i --fence --csv --only $2
    $BENCHMARK ./data/$1 ./data/$1_ops_20M_0.000000rq_0.500000nl_0.500000i_0m_10Mbulkload --fence --csv --only $2
    $BENCHMARK ./data/$1 ./data/$1_ops_360M_0.000000rq_0.500000nl_0.500000i_0m_9blk --fence --csv --only $2
}

mkdir -p ./results

for INDEX in ART RMI TS FAST PGM DynamicPGM LIPP BTree ALEX MABTree XIndex FINEdex Wormhole
do
    execute_uint64_200M wiki_ts_200M_uint64 $INDEX
    execute_uint64_200M fb_200M_uint64 $INDEX
    execute_uint64_200M osm_cellids_200M_uint64 $INDEX
    execute_uint64_200M books_200M_uint64 $INDEX
done
