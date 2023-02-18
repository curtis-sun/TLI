#include "benchmarks/benchmark_fast.h"

#include "benchmark.h"
#include "common.h"
#include "competitors/fast.h"

void benchmark_64_fast(tli::Benchmark<uint64_t>& benchmark) {
  benchmark.template Run<Fast<uint64_t>>();
}
