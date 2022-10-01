#pragma once
#include <string>

#include "benchmark.h"

// template <typename Searcher>
// void benchmark_32_rmi(sosd::Benchmark<uint32_t>& benchmark,
//                        bool pareto, const std::string& filename);

template <typename Searcher>
void benchmark_64_rmi(sosd::Benchmark<uint64_t>& benchmark, 
                      bool pareto, const std::vector<int>& params, const std::string& filename);

template <int record>
void benchmark_64_rmi(sosd::Benchmark<uint64_t>& benchmark, const std::string& filename);
