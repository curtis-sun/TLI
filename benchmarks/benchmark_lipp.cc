#include "benchmarks/benchmark_lipp.h"

#include "benchmark.h"
#include "common.h"
#include "competitors/lipp.h"

// void benchmark_32_lipp(sosd::Benchmark<uint32_t>& benchmark,
//                        bool pareto) {
//   benchmark.template Run<Lipp<uint32_t>>();
// }

void benchmark_64_lipp(sosd::Benchmark<uint64_t>& benchmark) {
  benchmark.template Run<Lipp<uint64_t>>();
}
