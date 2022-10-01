#include "benchmarks/benchmark_xindex.h"

#include "benchmark.h"
#include "benchmarks/common.h"
#include "competitors/xindex.h"

// template <typename Searcher>
// void benchmark_32_xindex(sosd::Benchmark<uint32_t>& benchmark,
//                       bool pareto) {
//   benchmark.template Run<sosd_xindex::XIndex<uint32_t, Searcher, 16>>();
//   if (pareto) {
//     benchmark.template Run<sosd_xindex::XIndex<uint32_t, Searcher, 32>>();
//   }
// }

// template <>
// void benchmark_32_xindex<LinearAVX<uint32_t, 0>>(sosd::Benchmark<uint32_t>& benchmark,
//                       bool pareto){}
// template <>
// void benchmark_32_xindex<LinearAVX<uint32_t, 1>>(sosd::Benchmark<uint32_t>& benchmark,
//                       bool pareto){}
// template <>
// void benchmark_32_xindex<LinearAVX<uint32_t, 2>>(sosd::Benchmark<uint32_t>& benchmark,
//                       bool pareto){}
// template <>
// void benchmark_32_xindex<InterpolationSearch<0>>(sosd::Benchmark<uint32_t>& benchmark,
//                       bool pareto){}
// template <>
// void benchmark_32_xindex<InterpolationSearch<1>>(sosd::Benchmark<uint32_t>& benchmark,
//                       bool pareto){}
// template <>
// void benchmark_32_xindex<InterpolationSearch<2>>(sosd::Benchmark<uint32_t>& benchmark,
//                       bool pareto){}

template <typename Searcher>
void benchmark_64_xindex(sosd::Benchmark<uint64_t>& benchmark, 
                         bool pareto, const std::vector<int>& params, const std::string& filename) {
  if (!pareto){
    benchmark.template Run<sosd_xindex::XIndex<uint64_t, Searcher>>(params);
  }
  else {
    if (filename.find("books_200M_uint64") != std::string::npos) {
      benchmark.template Run<sosd_xindex::XIndex<uint64_t, Searcher>>({8});
    }
    benchmark.template Run<sosd_xindex::XIndex<uint64_t, Searcher>>({16});
    benchmark.template Run<sosd_xindex::XIndex<uint64_t, Searcher>>({32});
    benchmark.template Run<sosd_xindex::XIndex<uint64_t, Searcher>>({64});
    benchmark.template Run<sosd_xindex::XIndex<uint64_t, Searcher>>({128});
    benchmark.template Run<sosd_xindex::XIndex<uint64_t, Searcher>>({256});
    benchmark.template Run<sosd_xindex::XIndex<uint64_t, Searcher>>({512});
  }
}

template <int record>
void benchmark_64_xindex(sosd::Benchmark<uint64_t>& benchmark, const std::string& filename) {
  if (filename.find("books_200M") != std::string::npos) {
    if (filename.find("0.000000i") != std::string::npos) {
      benchmark.template Run<sosd_xindex::XIndex<uint64_t, ExponentialSearch<record>>>({16});
      benchmark.template Run<sosd_xindex::XIndex<uint64_t, ExponentialSearch<record>>>({32});
      benchmark.template Run<sosd_xindex::XIndex<uint64_t, ExponentialSearch<record>>>({8});
    } else if (filename.find("mix") == std::string::npos) {
      benchmark.template Run<sosd_xindex::XIndex<uint64_t, ExponentialSearch<record>>>({16});
      benchmark.template Run<sosd_xindex::XIndex<uint64_t, ExponentialSearch<record>>>({32});
      benchmark.template Run<sosd_xindex::XIndex<uint64_t, ExponentialSearch<record>>>({8});
    } else {
      if (filename.find("0.050000i") != std::string::npos) {
        benchmark.template Run<sosd_xindex::XIndex<uint64_t, ExponentialSearch<record>>>({16});
        benchmark.template Run<sosd_xindex::XIndex<uint64_t, ExponentialSearch<record>>>({32});
        benchmark.template Run<sosd_xindex::XIndex<uint64_t, ExponentialSearch<record>>>({8});
      } else if (filename.find("0.500000i") != std::string::npos) {
        benchmark.template Run<sosd_xindex::XIndex<uint64_t, ExponentialSearch<record>>>({16});
        benchmark.template Run<sosd_xindex::XIndex<uint64_t, ExponentialSearch<record>>>({32});
        benchmark.template Run<sosd_xindex::XIndex<uint64_t, ExponentialSearch<record>>>({8});
      }  else if (filename.find("0.800000i") != std::string::npos) {
        benchmark.template Run<sosd_xindex::XIndex<uint64_t, ExponentialSearch<record>>>({16});
        benchmark.template Run<sosd_xindex::XIndex<uint64_t, ExponentialSearch<record>>>({32});
        benchmark.template Run<sosd_xindex::XIndex<uint64_t, ExponentialSearch<record>>>({8});
      }
    }
  }
  if (filename.find("fb_200M") != std::string::npos) {
    if (filename.find("0.000000i") != std::string::npos) {
      benchmark.template Run<sosd_xindex::XIndex<uint64_t, ExponentialSearch<record>>>({64});
      benchmark.template Run<sosd_xindex::XIndex<uint64_t, BranchingBinarySearch<record>>>({64});
      benchmark.template Run<sosd_xindex::XIndex<uint64_t, BranchingBinarySearch<record>>>({512});
    } else if (filename.find("mix") == std::string::npos) {
      benchmark.template Run<sosd_xindex::XIndex<uint64_t, ExponentialSearch<record>>>({32});
      if (filename.find("1.000000i") != std::string::npos){
        benchmark.template Run<sosd_xindex::XIndex<uint64_t, LinearSearch<record>>>({32});
      }
      benchmark.template Run<sosd_xindex::XIndex<uint64_t, ExponentialSearch<record>>>({64});
    } else {
      if (filename.find("0.050000i") != std::string::npos) {
        benchmark.template Run<sosd_xindex::XIndex<uint64_t, ExponentialSearch<record>>>({64});
        benchmark.template Run<sosd_xindex::XIndex<uint64_t, BranchingBinarySearch<record>>>({64});
        benchmark.template Run<sosd_xindex::XIndex<uint64_t, ExponentialSearch<record>>>({128});
      } else if (filename.find("0.500000i") != std::string::npos) {
        benchmark.template Run<sosd_xindex::XIndex<uint64_t, ExponentialSearch<record>>>({64});
        benchmark.template Run<sosd_xindex::XIndex<uint64_t, ExponentialSearch<record>>>({128});
        benchmark.template Run<sosd_xindex::XIndex<uint64_t, BranchingBinarySearch<record>>>({64});
      }  else if (filename.find("0.800000i") != std::string::npos) {
        benchmark.template Run<sosd_xindex::XIndex<uint64_t, ExponentialSearch<record>>>({32});
        benchmark.template Run<sosd_xindex::XIndex<uint64_t, ExponentialSearch<record>>>({64});
        benchmark.template Run<sosd_xindex::XIndex<uint64_t, ExponentialSearch<record>>>({128});
      }
    }
  }
  if (filename.find("osm_cellids_200M") != std::string::npos) {
    if (filename.find("0.000000i") != std::string::npos) {
      benchmark.template Run<sosd_xindex::XIndex<uint64_t, BranchingBinarySearch<record>>>({256});
      benchmark.template Run<sosd_xindex::XIndex<uint64_t, BranchingBinarySearch<record>>>({512});
      benchmark.template Run<sosd_xindex::XIndex<uint64_t, ExponentialSearch<record>>>({256});
    } else if (filename.find("mix") == std::string::npos) {
      benchmark.template Run<sosd_xindex::XIndex<uint64_t, BranchingBinarySearch<record>>>({256});
      benchmark.template Run<sosd_xindex::XIndex<uint64_t, ExponentialSearch<record>>>({128});
      benchmark.template Run<sosd_xindex::XIndex<uint64_t, ExponentialSearch<record>>>({256});
    } else {
      if (filename.find("0.050000i") != std::string::npos) {
        benchmark.template Run<sosd_xindex::XIndex<uint64_t, BranchingBinarySearch<record>>>({256});
        benchmark.template Run<sosd_xindex::XIndex<uint64_t, BranchingBinarySearch<record>>>({512});
        benchmark.template Run<sosd_xindex::XIndex<uint64_t, ExponentialSearch<record>>>({256});
      } else if (filename.find("0.500000i") != std::string::npos) {
        benchmark.template Run<sosd_xindex::XIndex<uint64_t, BranchingBinarySearch<record>>>({256});
        benchmark.template Run<sosd_xindex::XIndex<uint64_t, ExponentialSearch<record>>>({256});
        benchmark.template Run<sosd_xindex::XIndex<uint64_t, BranchingBinarySearch<record>>>({512});
      }  else if (filename.find("0.800000i") != std::string::npos) {
        benchmark.template Run<sosd_xindex::XIndex<uint64_t, BranchingBinarySearch<record>>>({256});
        benchmark.template Run<sosd_xindex::XIndex<uint64_t, ExponentialSearch<record>>>({128});
        benchmark.template Run<sosd_xindex::XIndex<uint64_t, ExponentialSearch<record>>>({256});
      }
    }
  }
}

template <>
void benchmark_64_xindex<LinearAVX<uint64_t, 0>>(sosd::Benchmark<uint64_t>&,
                      bool, const std::vector<int>&, const std::string&){}
template <>
void benchmark_64_xindex<LinearAVX<uint64_t, 1>>(sosd::Benchmark<uint64_t>&,
                      bool, const std::vector<int>&, const std::string&){}
template <>
void benchmark_64_xindex<LinearAVX<uint64_t, 2>>(sosd::Benchmark<uint64_t>&,
                      bool, const std::vector<int>&, const std::string&){}
template <>
void benchmark_64_xindex<InterpolationSearch<0>>(sosd::Benchmark<uint64_t>&,
                      bool, const std::vector<int>&, const std::string&){}
template <>
void benchmark_64_xindex<InterpolationSearch<1>>(sosd::Benchmark<uint64_t>&,
                      bool, const std::vector<int>&, const std::string&){}
template <>
void benchmark_64_xindex<InterpolationSearch<2>>(sosd::Benchmark<uint64_t>&,
                      bool, const std::vector<int>&, const std::string&){}

// INSTANTIATE_TEMPLATES_MULTITHREAD(benchmark_32_xindex, uint32_t);
INSTANTIATE_TEMPLATES_RMI_(benchmark_64_xindex, uint64_t, 0);
INSTANTIATE_TEMPLATES_RMI_(benchmark_64_xindex, uint64_t, 1);
INSTANTIATE_TEMPLATES_RMI_(benchmark_64_xindex, uint64_t, 2);