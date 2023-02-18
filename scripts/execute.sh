#! /usr/bin/env bash

echo "Executing benchmark and saving results..."

BENCHMARK=build/benchmark
if [ ! -f $BENCHMARK ]; then
    echo "benchmark binary does not exist"
    exit
fi

function execute_uint64_200M() {
    echo "Executing operations for $1 and index $2"
    $BENCHMARK ./data/$1 ./data/$1_ops_20M_0.000000rq_0.500000nl_0.000000i --through --csv --only $2 -r 3
    $BENCHMARK ./data/$1 ./data/$1_ops_20M_1.000000rq_0.000000nl_0.000000i --through --csv --only $2 -r 3

    $BENCHMARK ./data/$1 ./data/$1_ops_86666666_0.000000rq_0.500000nl_0.769231i_0m --through --csv --only $2 -r 3
    $BENCHMARK ./data/$1 ./data/$1_ops_201818181_0.000000rq_0.500000nl_0.900901i_0m --through --csv --only $2 -r 3
    $BENCHMARK ./data/$1 ./data/$1_ops_29523809_0.000000rq_0.500000nl_0.322581i_0m --through --csv --only $2 -r 3
    $BENCHMARK ./data/$1 ./data/$1_ops_207500K_0.000000rq_0.500000nl_0.903614i_0m --through --csv --only $2 -r 3
    $BENCHMARK ./data/$1 ./data/$1_ops_38181818_0.000000rq_0.500000nl_0.476190i_0m --through --csv --only $2 -r 3
    $BENCHMARK ./data/$1 ./data/$1_ops_120M_0.000000rq_0.500000nl_0.833333i_0m --through --csv --only $2 -r 3
    $BENCHMARK ./data/$1 ./data/$1_ops_186666666_0.000000rq_0.500000nl_0.892857i_0m --through --csv --only $2 -r 3

    $BENCHMARK ./data/$1 ./data/$1_ops_100M_0.000000rq_0.000000nl_1.000000i_0m_5Mbulkload --through --csv --only $2 -r 3
    $BENCHMARK ./data/$1 ./data/$1_ops_100M_0.010000rq_0.500000nl_0.800000i_0m_mix_5Mbulkload --through --csv --only $2 -r 3
    $BENCHMARK ./data/$1 ./data/$1_ops_100M_0.025000rq_0.500000nl_0.500000i_0m_mix_5Mbulkload --through --csv --only $2 -r 3
    $BENCHMARK ./data/$1 ./data/$1_ops_100M_0.047500rq_0.500000nl_0.050000i_0m_mix_5Mbulkload --through --csv --only $2 -r 3
    $BENCHMARK ./data/$1 ./data/$1_ops_100M_0.050000rq_0.500000nl_0.000000i_mix_105Mbulkload --through --csv --only $2 -r 3

    $BENCHMARK ./data/$1 ./data/$1_ops_20M_0.000000rq_0.500000nl_0.500000i_0m_10Mbulkload --through --csv --only $2 -r 3
    $BENCHMARK ./data/$1 ./data/$1_ops_20M_0.000000rq_0.500000nl_0.500000i_1m_10Mbulkload --through --csv --only $2 -r 3
    $BENCHMARK ./data/$1 ./data/$1_ops_20M_0.000000rq_0.500000nl_0.500000i_2m_0.050000h_10Mbulkload --through --csv --only $2 -r 3
}

function execute_string_90M() {
    echo "Executing operations for $1 and index $2"
    $BENCHMARK ./data/$1 ./data/$1_ops_60M_0.000000rq_0.000000nl_1.000000i_0m_3Mbulkload --through --csv --only $2 -r 3
    $BENCHMARK ./data/$1 ./data/$1_ops_60M_0.000000rq_0.000000nl_0.500000i_0m_mix_3Mbulkload --through --csv --only $2 -r 3
    $BENCHMARK ./data/$1 ./data/$1_ops_60M_0.000000rq_0.000000nl_0.050000i_0m_mix_3Mbulkload --through --csv --only $2 -r 3
    $BENCHMARK ./data/$1 ./data/$1_ops_60M_0.000000rq_0.000000nl_0.000000i_63Mbulkload --through --csv --only $2 -r 3
}

mkdir -p ./results

for DATA in fb osm_cellids books wiki_ts
do
for INDEX in ART RMI TS FAST PGM DynamicPGM LIPP BTree ALEX MABTree XIndex FINEdex Wormhole
do
    execute_uint64_200M ${DATA}_200M_uint64 $INDEX
done
done

for INDEX in ART Wormhole SIndex
do
    execute_string_90M url_90M_string $INDEX
done
