#include "benchmarks/benchmark_fast.h"

#include "benchmark.h"
#include "common.h"
#include "competitors/fast.h"

// void benchmark_32_fast(sosd::Benchmark<uint32_t>& benchmark,
//                        bool pareto) {
//   benchmark.template Run<Fast<uint32_t>>();
// }

void benchmark_64_fast(sosd::Benchmark<uint64_t>& benchmark) {
  benchmark.template Run<Fast<uint64_t>>();
}
