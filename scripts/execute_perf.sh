#! /usr/bin/env bash

echo "Executing benchmark and saving results..."

BENCHMARK=build/benchmark
if [ ! -f $BENCHMARK ]; then
    echo "benchmark binary does not exist"
    exit
fi

function execute_uint64_200M() {
    echo "Executing operations for dataset $1 and index $2"
    perf stat -e cpu-cycles,instructions $BENCHMARK ./data/$1 ./data/$1_ops_100M_0.000000rq_0.000000nl_1.000000i_0m_5Mbulkload --fence --only $2
    perf stat -e cpu-cycles,instructions $BENCHMARK ./data/$1 ./data/$1_ops_100M_0.010000rq_0.500000nl_0.800000i_0m_mix_5Mbulkload --fence --only $2
    perf stat -e cpu-cycles,instructions $BENCHMARK ./data/$1 ./data/$1_ops_100M_0.025000rq_0.500000nl_0.500000i_0m_mix_5Mbulkload --fence --only $2
    perf stat -e cpu-cycles,instructions $BENCHMARK ./data/$1 ./data/$1_ops_100M_0.047500rq_0.500000nl_0.050000i_0m_mix_5Mbulkload --fence --only $2
    perf stat -e cpu-cycles,instructions $BENCHMARK ./data/$1 ./data/$1_ops_100M_0.050000rq_0.500000nl_0.000000i_mix_105Mbulkload --fence --only $2

    python3 ~/pmu-tools/toplev.py --single-thread -l3 -v --no-desc --csv , --output ./results/$1_ops_100M_0.000000rq_0.000000nl_1.000000i_0m_5Mbulkload_$2_perf_table.csv $BENCHMARK ./data/$1 ./data/$1_ops_100M_0.000000rq_0.000000nl_1.000000i_0m_5Mbulkload --fence --only $2
    python3 ~/pmu-tools/toplev.py --single-thread -l3 -v --no-desc --csv , --output ./results/$1_ops_100M_0.010000rq_0.500000nl_0.800000i_0m_mix_5Mbulkload_$2_perf_table.csv $BENCHMARK ./data/$1 ./data/$1_ops_100M_0.010000rq_0.500000nl_0.800000i_0m_mix_5Mbulkload --fence --only $2
    python3 ~/pmu-tools/toplev.py --single-thread -l3 -v --no-desc --csv , --output ./results/$1_ops_100M_0.025000rq_0.500000nl_0.500000i_0m_mix_5Mbulkload_$2_perf_table.csv $BENCHMARK ./data/$1 ./data/$1_ops_100M_0.025000rq_0.500000nl_0.500000i_0m_mix_5Mbulkload --fence --only $2
    python3 ~/pmu-tools/toplev.py --single-thread -l3 -v --no-desc --csv , --output ./results/$1_ops_100M_0.047500rq_0.500000nl_0.050000i_0m_mix_5Mbulkload_$2_perf_table.csv $BENCHMARK ./data/$1 ./data/$1_ops_100M_0.047500rq_0.500000nl_0.050000i_0m_mix_5Mbulkload --fence --only $2
    python3 ~/pmu-tools/toplev.py --single-thread -l3 -v --no-desc --csv , --output ./results/$1_ops_100M_0.050000rq_0.500000nl_0.000000i_mix_105Mbulkload_$2_perf_table.csv $BENCHMARK ./data/$1 ./data/$1_ops_100M_0.050000rq_0.500000nl_0.000000i_mix_105Mbulkload --fence --only $2

    perf stat -e cpu-cycles,instructions $BENCHMARK ./data/$1 ./data/$1_ops_100M_0.000000rq_0.000000nl_1.000000i_0m_5Mbulkload --fence --only $2 --build
    perf stat -e cpu-cycles,instructions $BENCHMARK ./data/$1 ./data/$1_ops_100M_0.010000rq_0.500000nl_0.800000i_0m_mix_5Mbulkload --fence --only $2 --build
    perf stat -e cpu-cycles,instructions $BENCHMARK ./data/$1 ./data/$1_ops_100M_0.025000rq_0.500000nl_0.500000i_0m_mix_5Mbulkload --fence --only $2 --build
    perf stat -e cpu-cycles,instructions $BENCHMARK ./data/$1 ./data/$1_ops_100M_0.047500rq_0.500000nl_0.050000i_0m_mix_5Mbulkload --fence --only $2 --build
    perf stat -e cpu-cycles,instructions $BENCHMARK ./data/$1 ./data/$1_ops_100M_0.050000rq_0.500000nl_0.000000i_mix_105Mbulkload --fence --only $2 --build

    python3 ~/pmu-tools/toplev.py --single-thread -l3 -v --no-desc --csv , --output ./results/$1_ops_100M_0.000000rq_0.000000nl_1.000000i_0m_5Mbulkload_build_$2_perf_table.csv $BENCHMARK ./data/$1 ./data/$1_ops_100M_0.000000rq_0.000000nl_1.000000i_0m_5Mbulkload --fence --only $2 --build
    python3 ~/pmu-tools/toplev.py --single-thread -l3 -v --no-desc --csv , --output ./results/$1_ops_100M_0.010000rq_0.500000nl_0.800000i_0m_mix_5Mbulkload_build_$2_perf_table.csv $BENCHMARK ./data/$1 ./data/$1_ops_100M_0.010000rq_0.500000nl_0.800000i_0m_mix_5Mbulkload --fence --only $2 --build
    python3 ~/pmu-tools/toplev.py --single-thread -l3 -v --no-desc --csv , --output ./results/$1_ops_100M_0.025000rq_0.500000nl_0.500000i_0m_mix_5Mbulkload_build_$2_perf_table.csv $BENCHMARK ./data/$1 ./data/$1_ops_100M_0.025000rq_0.500000nl_0.500000i_0m_mix_5Mbulkload --fence --only $2 --build
    python3 ~/pmu-tools/toplev.py --single-thread -l3 -v --no-desc --csv , --output ./results/$1_ops_100M_0.047500rq_0.500000nl_0.050000i_0m_mix_5Mbulkload_build_$2_perf_table.csv $BENCHMARK ./data/$1 ./data/$1_ops_100M_0.047500rq_0.500000nl_0.050000i_0m_mix_5Mbulkload --fence --only $2 --build
    python3 ~/pmu-tools/toplev.py --single-thread -l3 -v --no-desc --csv , --output ./results/$1_ops_100M_0.050000rq_0.500000nl_0.000000i_mix_105Mbulkload_build_$2_perf_table.csv $BENCHMARK ./data/$1 ./data/$1_ops_100M_0.050000rq_0.500000nl_0.000000i_mix_105Mbulkload --fence --only $2 --build
}

mkdir -p ./results

for INDEX in ART DynamicPGM LIPP BTree ALEX MABTree XIndex FINEdex Wormhole
do
    execute_uint64_200M wiki_ts_200M_uint64 $INDEX
    execute_uint64_200M fb_200M_uint64 $INDEX
    execute_uint64_200M osm_cellids_200M_uint64 $INDEX
    execute_uint64_200M books_200M_uint64 $INDEX
done
