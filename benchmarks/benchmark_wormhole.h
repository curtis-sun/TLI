#pragma once
#include "benchmark.h"

void benchmark_string_wormhole(tli::Benchmark<std::string>& benchmark);

void benchmark_64_wormhole(tli::Benchmark<uint64_t>& benchmark);
