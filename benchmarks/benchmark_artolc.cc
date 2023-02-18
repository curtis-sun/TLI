#include "benchmark_artolc.h"

#include "benchmark.h"
#include "common.h"
#include "competitors/artolc.h"

void benchmark_64_artolc(tli::Benchmark<uint64_t>& benchmark) {
  benchmark.template Run<ARTOLC<uint64_t>>();
}

void benchmark_string_artolc(tli::Benchmark<std::string>& benchmark) {
  benchmark.template Run<ARTOLC<std::string>>();
}
