#pragma once
#include "benchmarks/benchmark_sindex.h"

#include "benchmark.h"
#include "benchmarks/common.h"
#include "competitors/sindex.h"

template <typename Searcher>
void benchmark_string_sindex(sosd::Benchmark<std::string>& benchmark, 
                             bool pareto, const std::vector<int>& params) {
  if (!pareto){
    benchmark.template Run<sosd_sindex::SIndex<std::string, Searcher>>(params);
  }
  else {
    benchmark.template Run<sosd_sindex::SIndex<std::string, Searcher>>({64, 4});
    benchmark.template Run<sosd_sindex::SIndex<std::string, Searcher>>({64, 8});
    benchmark.template Run<sosd_sindex::SIndex<std::string, Searcher>>({64, 16});
    benchmark.template Run<sosd_sindex::SIndex<std::string, Searcher>>({128, 4});
    benchmark.template Run<sosd_sindex::SIndex<std::string, Searcher>>({128, 8});
    benchmark.template Run<sosd_sindex::SIndex<std::string, Searcher>>({128, 16});
    benchmark.template Run<sosd_sindex::SIndex<std::string, Searcher>>({256, 8});
    benchmark.template Run<sosd_sindex::SIndex<std::string, Searcher>>({256, 16});
    benchmark.template Run<sosd_sindex::SIndex<std::string, Searcher>>({256, 32});
    benchmark.template Run<sosd_sindex::SIndex<std::string, Searcher>>({512, 16});
    benchmark.template Run<sosd_sindex::SIndex<std::string, Searcher>>({512, 32});
    benchmark.template Run<sosd_sindex::SIndex<std::string, Searcher>>({512, 64});
    benchmark.template Run<sosd_sindex::SIndex<std::string, Searcher>>({1024, 16});
    benchmark.template Run<sosd_sindex::SIndex<std::string, Searcher>>({1024, 32});
    benchmark.template Run<sosd_sindex::SIndex<std::string, Searcher>>({1024, 64});
  }
}

template <int record>
void benchmark_string_sindex(sosd::Benchmark<std::string>& benchmark, const std::string& filename) {
  if (filename.find("url_90M") != std::string::npos) {
    if (filename.find("0.000000i") != std::string::npos) {
      benchmark.template Run<sosd_sindex::SIndex<std::string, BranchingBinarySearch<record>>>({256,8});
      benchmark.template Run<sosd_sindex::SIndex<std::string, BranchingBinarySearch<record>>>({128,4});
      benchmark.template Run<sosd_sindex::SIndex<std::string, BranchingBinarySearch<record>>>({128,8});
    } else if (filename.find("mix") == std::string::npos) {
      if (filename.find("0m") != std::string::npos) {
        benchmark.template Run<sosd_sindex::SIndex<std::string, BranchingBinarySearch<record>>>({64,4});
        benchmark.template Run<sosd_sindex::SIndex<std::string, BranchingBinarySearch<record>>>({128,4});
        benchmark.template Run<sosd_sindex::SIndex<std::string, ExponentialSearch<record>>>({64,4});
      }
    } else {
      if (filename.find("0.050000i") != std::string::npos) {
        benchmark.template Run<sosd_sindex::SIndex<std::string, BranchingBinarySearch<record>>>({256,8});
        benchmark.template Run<sosd_sindex::SIndex<std::string, BranchingBinarySearch<record>>>({128,4});
        benchmark.template Run<sosd_sindex::SIndex<std::string, BranchingBinarySearch<record>>>({128,8});
      } else if (filename.find("0.500000i") != std::string::npos) {
        benchmark.template Run<sosd_sindex::SIndex<std::string, BranchingBinarySearch<record>>>({128,4});
        benchmark.template Run<sosd_sindex::SIndex<std::string, ExponentialSearch<record>>>({64,4});
        benchmark.template Run<sosd_sindex::SIndex<std::string, BranchingBinarySearch<record>>>({64,4});
      }
    }
  }
}

template <>
void benchmark_string_sindex<LinearAVX<std::string, 0>>(sosd::Benchmark<std::string>& benchmark,
                      bool pareto, const std::vector<int>& params){}
template <>
void benchmark_string_sindex<LinearAVX<std::string, 1>>(sosd::Benchmark<std::string>& benchmark,
                      bool pareto, const std::vector<int>& params){}
template <>
void benchmark_string_sindex<LinearAVX<std::string, 2>>(sosd::Benchmark<std::string>& benchmark,
                      bool pareto, const std::vector<int>& params){}
template <>
void benchmark_string_sindex<InterpolationSearch<0>>(sosd::Benchmark<std::string>& benchmark,
                      bool pareto, const std::vector<int>& params){}
template <>
void benchmark_string_sindex<InterpolationSearch<1>>(sosd::Benchmark<std::string>& benchmark,
                      bool pareto, const std::vector<int>& params){}
template <>
void benchmark_string_sindex<InterpolationSearch<2>>(sosd::Benchmark<std::string>& benchmark,
                      bool pareto, const std::vector<int>& params){}

INSTANTIATE_TEMPLATES_MULTITHREAD(benchmark_string_sindex, std::string);
