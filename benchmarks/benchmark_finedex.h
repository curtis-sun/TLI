#pragma once
#include "benchmark.h"

template <typename Searcher>
void benchmark_64_finedex(tli::Benchmark<uint64_t>& benchmark, bool pareto, const std::vector<int>& params, const std::string& filename);

template <int record>
void benchmark_64_finedex(tli::Benchmark<uint64_t>& benchmark, const std::string& filename);
