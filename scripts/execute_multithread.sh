#! /usr/bin/env bash

echo "Executing benchmark and saving results..."
while getopts n:c arg; do
    case $arg in
        c) do_csv=true;;
    esac
done

BENCHMARK=build/benchmark
if [ ! -f $BENCHMARK ]; then
    echo "benchmark binary does not exist"
    exit
fi

function execute_lookups_200M() {
    echo "Executing lookups for dataset $1, $2 threads and index $3"
    $BENCHMARK ./data/$1 ./data/$1_equality_sqls_20M_0.000000rq_0.500000nl_0.000000i_$2t --through --threads $2 --csv --only $3
    $BENCHMARK ./data/$1 ./data/$1_equality_sqls_29523809_0.000000rq_0.500000nl_0.322581i_0m_$2t --through --threads $2 --csv --only $3
    $BENCHMARK ./data/$1 ./data/$1_equality_sqls_207500K_0.000000rq_0.500000nl_0.903614i_0m_$2t --through --threads $2 --csv --only $3
    $BENCHMARK ./data/$1 ./data/$1_equality_sqls_38181818_0.000000rq_0.500000nl_0.476190i_0m_$2t --through --threads $2 --csv --only $3
    $BENCHMARK ./data/$1 ./data/$1_equality_sqls_86666666_0.000000rq_0.500000nl_0.769231i_0m_$2t --through --threads $2 --csv --only $3
    $BENCHMARK ./data/$1 ./data/$1_equality_sqls_120M_0.000000rq_0.500000nl_0.833333i_0m_$2t --through --threads $2 --csv --only $3
    $BENCHMARK ./data/$1 ./data/$1_equality_sqls_186666666_0.000000rq_0.500000nl_0.892857i_0m_$2t --through --threads $2 --csv --only $3
    $BENCHMARK ./data/$1 ./data/$1_equality_sqls_201818181_0.000000rq_0.500000nl_0.900901i_0m_$2t --through --threads $2 --csv --only $3
}

function execute_lookups_200M_write_ratio() {
    echo "Executing lookups for dataset $1, $2 threads and index $3"
    $BENCHMARK ./data/$1 ./data/$1_equality_sqls_100M_0.000000rq_0.000000nl_1.000000i_0m_$2t_5Mbulkload --through --threads $2 --csv --only $3
    $BENCHMARK ./data/$1 ./data/$1_equality_sqls_100M_0.000000rq_0.500000nl_0.500000i_0m_$2t_mix_5Mbulkload --through --threads $2 --csv --only $3
    $BENCHMARK ./data/$1 ./data/$1_equality_sqls_100M_0.000000rq_0.500000nl_0.050000i_0m_$2t_mix_5Mbulkload --through --threads $2 --csv --only $3
    $BENCHMARK ./data/$1 ./data/$1_equality_sqls_100M_0.000000rq_0.500000nl_0.000000i_$2t_105Mbulkload --through --threads $2 --csv --only $3
    # $BENCHMARK ./data/$1 ./data/$1_equality_sqls_20M_1.000000rq_0.000000nl_0.000000i_$2t --through --threads $2 --csv --only $3
}

function execute_strings_90M_write_ratio() {
    echo "Executing lookups for dataset $1, $2 threads and index $3"
    $BENCHMARK ./data/$1 ./data/$1_equality_sqls_60M_0.000000rq_0.000000nl_1.000000i_0m_$2t_3Mbulkload --threads $2 --through --csv --only $3
    $BENCHMARK ./data/$1 ./data/$1_equality_sqls_60M_0.000000rq_0.000000nl_0.500000i_0m_$2t_mix_3Mbulkload --threads $2 --through --csv --only $3
    $BENCHMARK ./data/$1 ./data/$1_equality_sqls_60M_0.000000rq_0.000000nl_0.050000i_0m_$2t_mix_3Mbulkload --threads $2 --through --csv --only $3
    $BENCHMARK ./data/$1 ./data/$1_equality_sqls_60M_0.000000rq_0.000000nl_0.000000i_$2t_63Mbulkload --threads $2 --through --csv --only $3
}

mkdir -p ./results

for DATA in fb osm_cellids books
do
for INDEX in XIndex FINEdex Wormhole
do
execute_lookups_200M ${DATA}_200M_uint64 24 $INDEX

for THREAD in 2 4 8 16 24 32
do
    execute_lookups_200M_write_ratio ${DATA}_200M_uint64 $THREAD $INDEX
done
done
done

for INDEX in Wormhole SIndex
do
for THREAD in 2 4 8 16 24 32
do
    execute_strings_90M_write_ratio url_90M_string $THREAD $INDEX
done 
done
