#include "benchmarks/benchmark_wormhole.h"

#include "benchmark.h"
#include "common.h"
#include "competitors/wormhole.h"

void benchmark_string_wormhole(tli::Benchmark<std::string>& benchmark) {
  benchmark.template Run<Wormhole<std::string>>();
}

void benchmark_64_wormhole(tli::Benchmark<uint64_t>& benchmark){
  benchmark.template Run<Wormhole<uint64_t>>();                          
}
