#include "benchmarks/benchmark_pgm.h"

#include "benchmark.h"
#include "benchmarks/common.h"
#include "competitors/pgm_index.h"

// template <typename Searcher>
// void benchmark_32_pgm(sosd::Benchmark<uint32_t>& benchmark,
//                       bool pareto) {
//   benchmark.template Run<PGM<uint32_t, Searcher, 16>>();
//   if (pareto) {
//     benchmark.template Run<PGM<uint32_t, Searcher, 4>>();
//     benchmark.template Run<PGM<uint32_t, Searcher, 8>>();
//     benchmark.template Run<PGM<uint32_t, Searcher, 32>>();
//     benchmark.template Run<PGM<uint32_t, Searcher, 128>>();
//   }
// }

template <typename Searcher>
void benchmark_64_pgm(sosd::Benchmark<uint64_t>& benchmark, 
                      bool pareto, const std::vector<int>& params) {
  if (!pareto){
    util::fail("PGM's hyperparameter cannot be set");
  }
  else {
    benchmark.template Run<PGM<uint64_t, Searcher, 4>>();
    benchmark.template Run<PGM<uint64_t, Searcher, 8>>();
    benchmark.template Run<PGM<uint64_t, Searcher, 16>>();
    benchmark.template Run<PGM<uint64_t, Searcher, 32>>();
    benchmark.template Run<PGM<uint64_t, Searcher, 64>>();
    benchmark.template Run<PGM<uint64_t, Searcher, 128>>();
    benchmark.template Run<PGM<uint64_t, Searcher, 256>>();
  }
}

template <int record>
void benchmark_64_pgm(sosd::Benchmark<uint64_t>& benchmark, const std::string& filename) {
  if (filename.find("books_200M") != std::string::npos) {
    benchmark.template Run<PGM<uint64_t, LinearSearch<record>,32>>();
    benchmark.template Run<PGM<uint64_t, BranchingBinarySearch<record>,64>>();
    benchmark.template Run<PGM<uint64_t, BranchingBinarySearch<record>,256>>();
  }
  if (filename.find("fb_200M") != std::string::npos) {
    benchmark.template Run<PGM<uint64_t, LinearSearch<record>,32>>();
    benchmark.template Run<PGM<uint64_t, BranchingBinarySearch<record>,128>>();
    benchmark.template Run<PGM<uint64_t, BranchingBinarySearch<record>,256>>();
  }
  if (filename.find("osm_cellids_200M") != std::string::npos) {
    benchmark.template Run<PGM<uint64_t, BranchingBinarySearch<record>,64>>();
    benchmark.template Run<PGM<uint64_t, BranchingBinarySearch<record>,128>>();
    benchmark.template Run<PGM<uint64_t, LinearSearch<record>,64>>();
  }
  if (filename.find("wiki_ts_200M") != std::string::npos) {
    benchmark.template Run<PGM<uint64_t, LinearSearch<record>,32>>();
    benchmark.template Run<PGM<uint64_t, ExponentialSearch<record>,8>>();
    benchmark.template Run<PGM<uint64_t, BranchingBinarySearch<record>,64>>();
  }
}

// INSTANTIATE_TEMPLATES(benchmark_32_pgm, uint32_t);
INSTANTIATE_TEMPLATES(benchmark_64_pgm, uint64_t);