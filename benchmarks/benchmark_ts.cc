#include "benchmarks/benchmark_ts.h"

#include "benchmark.h"
#include "benchmarks/common.h"
#include "competitors/ts.h"

template <typename Searcher>
void benchmark_64_ts(tli::Benchmark<uint64_t>& benchmark, 
                     bool pareto, const std::vector<int>& params) {
  if (!pareto){
    benchmark.template Run<TS<uint64_t, Searcher>>(params);
  }
  else {
    benchmark.template Run<TS<uint64_t, Searcher>>({8});
    benchmark.template Run<TS<uint64_t, Searcher>>({16});
    benchmark.template Run<TS<uint64_t, Searcher>>({32});
    benchmark.template Run<TS<uint64_t, Searcher>>({64});
    benchmark.template Run<TS<uint64_t, Searcher>>({128});
    benchmark.template Run<TS<uint64_t, Searcher>>({256});
    benchmark.template Run<TS<uint64_t, Searcher>>({512});
  }
}

template <int record>
void benchmark_64_ts(tli::Benchmark<uint64_t>& benchmark, const std::string& filename) {
  if (filename.find("books_200M") != std::string::npos) {
    benchmark.template Run<TS<uint64_t, LinearSearch<record>>>({8});
    benchmark.template Run<TS<uint64_t, LinearSearch<record>>>({16});
    benchmark.template Run<TS<uint64_t, BranchingBinarySearch<record>>>({16});
  }
  if (filename.find("fb_200M") != std::string::npos) {
    benchmark.template Run<TS<uint64_t, LinearSearch<record>>>({8});
    benchmark.template Run<TS<uint64_t, LinearSearch<record>>>({16});
    benchmark.template Run<TS<uint64_t, BranchingBinarySearch<record>>>({8});
  }
  if (filename.find("osm_cellids_200M") != std::string::npos) {
    benchmark.template Run<TS<uint64_t, BranchingBinarySearch<record>>>({16});
    benchmark.template Run<TS<uint64_t, LinearSearch<record>>>({8});
    benchmark.template Run<TS<uint64_t, LinearSearch<record>>>({16});
  }
  if (filename.find("wiki_ts_200M") != std::string::npos) {
    benchmark.template Run<TS<uint64_t, LinearSearch<record>>>({8});
    benchmark.template Run<TS<uint64_t, LinearSearch<record>>>({16});
    benchmark.template Run<TS<uint64_t, BranchingBinarySearch<record>>>({32});
  }
}

INSTANTIATE_TEMPLATES(benchmark_64_ts, uint64_t);
