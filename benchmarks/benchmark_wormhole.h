#pragma once
#include "benchmark.h"

void benchmark_string_wormhole(sosd::Benchmark<std::string>& benchmark);

void benchmark_64_wormhole(sosd::Benchmark<uint64_t>& benchmark);

// void benchmark_32_wormhole(sosd::Benchmark<uint32_t>& benchmark,
//                            bool pareto);
