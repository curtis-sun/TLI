#pragma once
#include "benchmark.h"

template <typename Searcher>
void benchmark_string_sindex(tli::Benchmark<std::string>& benchmark, 
                             bool pareto, const std::vector<int>& params);

template <int record>
void benchmark_string_sindex(tli::Benchmark<std::string>& benchmark, const std::string& filename);
