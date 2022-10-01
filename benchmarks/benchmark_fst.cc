#include "benchmarks/benchmark_fst.h"

#include "benchmark.h"
#include "common.h"
#include "competitors/fst_wrapper.h"

// void benchmark_32_fst(sosd::Benchmark<uint32_t>& benchmark,
//                       bool pareto) {
//   benchmark.template Run<FST<uint32_t, 32>>();
//   if (pareto) {
//     benchmark.template Run<FST<uint32_t, 16>>();
//     benchmark.template Run<FST<uint32_t, 64>>();
//     benchmark.template Run<FST<uint32_t, 256>>();
//   }
// }

void benchmark_64_fst(sosd::Benchmark<uint64_t>& benchmark, 
                      bool pareto, const std::vector<int>& params) {
  if (!pareto){
    benchmark.template Run<FST<uint64_t>>(params);
  }
  else {
    benchmark.template Run<FST<uint64_t>>({8});
    benchmark.template Run<FST<uint64_t>>({16});
    benchmark.template Run<FST<uint64_t>>({32});
    benchmark.template Run<FST<uint64_t>>({64});
    benchmark.template Run<FST<uint64_t>>({128});
    benchmark.template Run<FST<uint64_t>>({256});
    benchmark.template Run<FST<uint64_t>>({512});
  }
}

void benchmark_64_fst(sosd::Benchmark<uint64_t>& benchmark, const std::string& filename) {
  if (filename.find("books_200M") != std::string::npos) {
    benchmark.template Run<FST<uint64_t>>({8});
    benchmark.template Run<FST<uint64_t>>({16});
    benchmark.template Run<FST<uint64_t>>({512});
  }
  if (filename.find("fb_200M") != std::string::npos) {
    benchmark.template Run<FST<uint64_t>>({32});
    benchmark.template Run<FST<uint64_t>>({16});
    benchmark.template Run<FST<uint64_t>>({8});
  }
  if (filename.find("osm_cellids_200M") != std::string::npos) {
    benchmark.template Run<FST<uint64_t>>({16});
    benchmark.template Run<FST<uint64_t>>({32});
    benchmark.template Run<FST<uint64_t>>({8});
  } 
}

void benchmark_string_fst(sosd::Benchmark<std::string>& benchmark, 
                          bool pareto, const std::vector<int>& params) {
  if (!pareto){
    benchmark.template Run<FST<std::string>>(params);
  }
  else {
    benchmark.template Run<FST<std::string>>({16});
    benchmark.template Run<FST<std::string>>({32});
    benchmark.template Run<FST<std::string>>({64});
    benchmark.template Run<FST<std::string>>({128});
    benchmark.template Run<FST<std::string>>({256});
    benchmark.template Run<FST<std::string>>({512});
    benchmark.template Run<FST<std::string>>({1024});
  }
}

void benchmark_string_fst(sosd::Benchmark<std::string>& benchmark, const std::string& filename) {
  if (filename.find("url_90M") != std::string::npos) {
    benchmark.template Run<FST<std::string>>({16});
    benchmark.template Run<FST<std::string>>({32});
    benchmark.template Run<FST<std::string>>({128});
  }
}
