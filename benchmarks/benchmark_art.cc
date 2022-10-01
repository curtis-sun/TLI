#include "benchmark_art.h"

#include "benchmark.h"
#include "common.h"
#include "competitors/art.h"

// void benchmark_32_art(sosd::Benchmark<uint32_t>& benchmark,
//                       bool pareto) {
//   benchmark.template Run<sosd_art::ART<uint32_t>>();
// }

void benchmark_64_art(sosd::Benchmark<uint64_t>& benchmark) {
  benchmark.template Run<sosd_art::ART<uint64_t>>();
}

void benchmark_string_art(sosd::Benchmark<std::string>& benchmark) {
  benchmark.template Run<sosd_art::ART<std::string>>();
}
