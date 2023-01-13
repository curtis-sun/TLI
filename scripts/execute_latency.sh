#! /usr/bin/env bash

echo "Executing benchmark and saving results..."

BENCHMARK=build/benchmark
if [ ! -f $BENCHMARK ]; then
    echo "benchmark binary does not exist"
    exit
fi

function execute_lookups_200M() {
    echo "Executing lookups for dataset $1 and index $2"
    $BENCHMARK ./data/$1 ./data/$1_ops_20M_0.000000rq_0.500000nl_0.000000i --fence --csv --only $2
    $BENCHMARK ./data/$1 ./data/$1_ops_20M_0.000000rq_0.500000nl_0.500000i_0m_10Mbulkload --fence --csv --only $2
}

mkdir -p ./results

for INDEX in ART RMI TS FAST PGM DynamicPGM LIPP BTree ALEX MABTree XIndex FINEdex Wormhole
do
    execute_lookups_200M wiki_ts_200M_uint64 $INDEX
    execute_lookups_200M fb_200M_uint64 $INDEX
    execute_lookups_200M osm_cellids_200M_uint64 $INDEX
    execute_lookups_200M books_200M_uint64 $INDEX
done

# execute_strings_50M url_50M_string
# execute_lookups_25M url_25M_string 0
# execute_strings_12500K url_12500K_string
# execute_strings_6250K url_6250K_string
# execute_strings_3125K url_3125K_string

# execute_lookups_800M osm_cellids_800M_uint64 0.5
# execute_lookups_800M books_800M_uint64 0.5
