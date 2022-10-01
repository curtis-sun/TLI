#include "benchmarks/benchmark_wormhole.h"

#include "benchmark.h"
#include "common.h"
#include "competitors/wormhole.h"

void benchmark_string_wormhole(sosd::Benchmark<std::string>& benchmark) {
  benchmark.template Run<Wormhole<std::string>>();
}

void benchmark_64_wormhole(sosd::Benchmark<uint64_t>& benchmark){
  benchmark.template Run<Wormhole<uint64_t>>();                          
}

// void benchmark_32_wormhole(sosd::Benchmark<uint32_t>& benchmark,
//                            bool pareto){
//   benchmark.template Run<Wormhole<uint32_t>>();                          
//                            }
