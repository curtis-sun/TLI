#pragma once
#include "benchmark.h"

// template <typename Searcher>
// void benchmark_32_mabtree(sosd::Benchmark<uint32_t>& benchmark,
//                          bool pareto);

template <typename Searcher>
void benchmark_64_mabtree(sosd::Benchmark<uint64_t>& benchmark, 
                          bool pareto, const std::vector<int>& params);

template <int record>
void benchmark_64_mabtree(sosd::Benchmark<uint64_t>& benchmark, const std::string& filename);
