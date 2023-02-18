#include "benchmark_art.h"

#include "benchmark.h"
#include "common.h"
#include "competitors/art.h"

void benchmark_64_art(tli::Benchmark<uint64_t>& benchmark) {
  benchmark.template Run<tli_art::ART<uint64_t>>();
}

void benchmark_string_art(tli::Benchmark<std::string>& benchmark) {
  benchmark.template Run<tli_art::ART<std::string>>();
}
