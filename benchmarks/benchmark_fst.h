#pragma once
#include "benchmark.h"

// void benchmark_32_fst(sosd::Benchmark<uint32_t>& benchmark,
//                       bool pareto);

void benchmark_64_fst(sosd::Benchmark<uint64_t>& benchmark, 
                      bool pareto, const std::vector<int>& params);

void benchmark_64_fst(sosd::Benchmark<uint64_t>& benchmark, const std::string& filename);

void benchmark_string_fst(sosd::Benchmark<std::string>& benchmark, 
                          bool pareto, const std::vector<int>& params);

void benchmark_string_fst(sosd::Benchmark<std::string>& benchmark, const std::string& filename);
