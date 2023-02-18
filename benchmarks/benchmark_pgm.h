#pragma once
#include "benchmark.h"

template <typename Searcher>
void benchmark_64_pgm(tli::Benchmark<uint64_t>& benchmark, 
                      bool pareto, const std::vector<int>& params);

template <int record>
void benchmark_64_pgm(tli::Benchmark<uint64_t>& benchmark, const std::string& filename);
