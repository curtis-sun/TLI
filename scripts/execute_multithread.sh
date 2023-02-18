#! /usr/bin/env bash

echo "Executing benchmark and saving results..."

BENCHMARK=build/benchmark
if [ ! -f $BENCHMARK ]; then
    echo "benchmark binary does not exist"
    exit
fi

function execute_uint64_200M() {
    echo "Executing operations for dataset $1, $2 threads and index $3"
    $BENCHMARK ./data/$1 ./data/$1_ops_29523809_0.000000rq_0.500000nl_0.322581i_0m_$2t --through --threads $2 --csv --only $3 -r 3
    $BENCHMARK ./data/$1 ./data/$1_ops_207500K_0.000000rq_0.500000nl_0.903614i_0m_$2t --through --threads $2 --csv --only $3 -r 3
    $BENCHMARK ./data/$1 ./data/$1_ops_38181818_0.000000rq_0.500000nl_0.476190i_0m_$2t --through --threads $2 --csv --only $3 -r 3
    $BENCHMARK ./data/$1 ./data/$1_ops_86666666_0.000000rq_0.500000nl_0.769231i_0m_$2t --through --threads $2 --csv --only $3 -r 3
    $BENCHMARK ./data/$1 ./data/$1_ops_120M_0.000000rq_0.500000nl_0.833333i_0m_$2t --through --threads $2 --csv --only $3 -r 3
    $BENCHMARK ./data/$1 ./data/$1_ops_186666666_0.000000rq_0.500000nl_0.892857i_0m_$2t --through --threads $2 --csv --only $3 -r 3
    $BENCHMARK ./data/$1 ./data/$1_ops_201818181_0.000000rq_0.500000nl_0.900901i_0m_$2t --through --threads $2 --csv --only $3 -r 3

    $BENCHMARK ./data/$1 ./data/$1_ops_20M_0.000000rq_0.500000nl_0.500000i_2m_0.050000h_$2t_10Mbulkload --through --threads $2 --csv --only $3 -r 3
    $BENCHMARK ./data/$1 ./data/$1_ops_20M_0.000000rq_0.500000nl_0.500000i_2m_0.050005h_$2t_10Mbulkload --through --threads $2 --csv --only $3 -r 3
    $BENCHMARK ./data/$1 ./data/$1_ops_20M_0.000000rq_0.500000nl_0.500000i_2m_0.050010h_$2t_10Mbulkload --through --threads $2 --csv --only $3 -r 3
    $BENCHMARK ./data/$1 ./data/$1_ops_20M_0.000000rq_0.500000nl_0.500000i_2m_0.050015h_$2t_10Mbulkload --through --threads $2 --csv --only $3 -r 3
    $BENCHMARK ./data/$1 ./data/$1_ops_20M_0.000000rq_0.500000nl_0.500000i_2m_0.050020h_$2t_10Mbulkload --through --threads $2 --csv --only $3 -r 3
    $BENCHMARK ./data/$1 ./data/$1_ops_20M_0.000000rq_0.500000nl_0.500000i_2m_0.050025h_$2t_10Mbulkload --through --threads $2 --csv --only $3 -r 3
    $BENCHMARK ./data/$1 ./data/$1_ops_20M_0.000000rq_0.500000nl_0.500000i_2m_0.050030h_$2t_10Mbulkload --through --threads $2 --csv --only $3 -r 3
}

function execute_uint64_200M_write_ratio() {
    echo "Executing operations for dataset $1, $2 threads and index $3"
    $BENCHMARK ./data/$1 ./data/$1_ops_100M_0.000000rq_0.000000nl_1.000000i_0m_$2t_5Mbulkload --through --threads $2 --csv --only $3 -r 3
    $BENCHMARK ./data/$1 ./data/$1_ops_100M_0.000000rq_0.500000nl_0.500000i_0m_$2t_mix_5Mbulkload --through --threads $2 --csv --only $3 -r 3
    $BENCHMARK ./data/$1 ./data/$1_ops_100M_0.000000rq_0.500000nl_0.050000i_0m_$2t_mix_5Mbulkload --through --threads $2 --csv --only $3 -r 3
    $BENCHMARK ./data/$1 ./data/$1_ops_100M_0.000000rq_0.500000nl_0.000000i_$2t_105Mbulkload --through --threads $2 --csv --only $3 -r 3
    $BENCHMARK ./data/$1 ./data/$1_ops_100M_0.950000rq_0.000000nl_0.050000i_0m_$2t_mix_5Mbulkload --through --threads $2 --csv --only $3 -r 3
}

function execute_string_90M_write_ratio() {
    echo "Executing operations for dataset $1, $2 threads and index $3"
    $BENCHMARK ./data/$1 ./data/$1_ops_60M_0.000000rq_0.000000nl_1.000000i_0m_$2t_3Mbulkload --threads $2 --through --csv --only $3 -r 3
    $BENCHMARK ./data/$1 ./data/$1_ops_60M_0.000000rq_0.000000nl_0.500000i_0m_$2t_mix_3Mbulkload --threads $2 --through --csv --only $3 -r 3
    $BENCHMARK ./data/$1 ./data/$1_ops_60M_0.000000rq_0.000000nl_0.050000i_0m_$2t_mix_3Mbulkload --threads $2 --through --csv --only $3 -r 3
    $BENCHMARK ./data/$1 ./data/$1_ops_60M_0.000000rq_0.000000nl_0.000000i_$2t_63Mbulkload --threads $2 --through --csv --only $3 -r 3
}

mkdir -p ./results

for DATA in fb osm_cellids books
do
for INDEX in ARTOLC XIndex FINEdex Wormhole
do
execute_uint64_200M ${DATA}_200M_uint64 24 $INDEX

for THREAD in 2 4 8 16 24 32
do
    execute_uint64_200M_write_ratio ${DATA}_200M_uint64 $THREAD $INDEX
done
done
done

for INDEX in ARTOLC Wormhole SIndex
do
for THREAD in 2 4 8 16 24 32
do
    execute_string_90M_write_ratio url_90M_string $THREAD $INDEX
done 
done
