#pragma once
#include "benchmark.h"

// void benchmark_32_art(sosd::Benchmark<uint32_t>& benchmark,
//                       bool pareto);

void benchmark_64_art(sosd::Benchmark<uint64_t>& benchmark);

void benchmark_string_art(sosd::Benchmark<std::string>& benchmark);
