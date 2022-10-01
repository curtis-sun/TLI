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
    echo "Executing lookups for dataset $1, index $3, and search method $2"
    $BENCHMARK ./data/$1 ./data/$1_equality_sqls_1M_0.000000rq_0.500000nl_0.000000i --through --pareto --search $2 --only $3 --csv
    $BENCHMARK ./data/$1 ./data/$1_equality_sqls_1M_0.000000rq_0.000000nl_1.000000i_0m --through --pareto --search $2 --only $3 --csv
    $BENCHMARK ./data/$1 ./data/$1_equality_sqls_1M_0.000000rq_0.000000nl_1.000000i_1m --through --pareto --search $2 --only $3 --csv
    $BENCHMARK ./data/$1 ./data/$1_equality_sqls_1M_0.000000rq_0.000000nl_1.000000i_2m_0.006000h --through --pareto --search $2 --only $3 --csv
}

function execute_lookups_200M_multithread() {
    echo "Executing lookups for dataset $1, index $3, and search method $2"
    $BENCHMARK ./data/$1 ./data/$1_equality_sqls_1M_0.000000rq_0.500000nl_0.000000i_24t --through --pareto --search $2 --only $3 --csv --threads 24
    $BENCHMARK ./data/$1 ./data/$1_equality_sqls_1M_0.000000rq_0.000000nl_1.000000i_0m_24t --through --pareto --search $2 --only $3 --csv --threads 24
}

function execute_string_90M() {
    echo "Executing lookups for dataset $1, index $3, and search method $2"
    $BENCHMARK ./data/$1 ./data/$1_equality_sqls_1M_0.000000rq_0.000000nl_0.000000i_10Mbulkload --through --pareto --search $2 --only $3 --csv
    $BENCHMARK ./data/$1 ./data/$1_equality_sqls_1M_0.000000rq_0.000000nl_1.000000i_0m_9Mbulkload --through --pareto --search $2 --only $3 --csv
}

function execute_string_90M_multithread() {
    echo "Executing lookups for dataset $1, index $3, and search method $2"
    $BENCHMARK ./data/$1 ./data/$1_equality_sqls_1M_0.000000rq_0.000000nl_0.000000i_24t_10Mbulkload1 --through --pareto --search $2 --only $3 --csv --threads 24
    $BENCHMARK ./data/$1 ./data/$1_equality_sqls_1M_0.000000rq_0.000000nl_1.000000i_0m_24t_9Mbulkload --through --pareto --search $2 --only $3 --csv --threads 24
}

mkdir -p ./results

for DATA in fb osm_cellids books wiki_ts
do
for INDEX in RMI TS PGM DynamicPGM BTree ALEX MABTree
do
for SEARCH in binary exponential linear avx interpolation
do
    execute_lookups_200M ${DATA}_200M_uint64 $SEARCH $INDEX
done
done
done

for DATA in fb osm_cellids books
do
for INDEX in XIndex FINEdex
do
for SEARCH in binary exponential linear avx interpolation
do
    execute_lookups_200M_multithread ${DATA}_200M_uint64 $SEARCH $INDEX
done
done
done

for SEARCH in binary exponential linear
do
    execute_string_90M_multithread url_90M_string $SEARCH SIndex
done

