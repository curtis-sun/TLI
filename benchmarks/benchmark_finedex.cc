#include "benchmarks/benchmark_xindex.h"

#include "benchmark.h"
#include "benchmarks/common.h"
#include "competitors/finedex.h"

// template <typename Searcher>
// void benchmark_32_finedex(sosd::Benchmark<uint32_t>& benchmark,
//                       bool pareto, const std::string& filename) {
//   benchmark.template Run<sosd_finedex::FINEdex<uint32_t, Searcher, 512>>();
//   if (filename.find("/books") == std::string::npos) {
//     benchmark.template Run<sosd_finedex::FINEdex<uint32_t, Searcher, 2048>>();
//     if (pareto){
//       benchmark.template Run<sosd_finedex::FINEdex<uint32_t, Searcher, 1024>>();
//     }
//   }
// }

template <typename Searcher>
void benchmark_64_finedex(sosd::Benchmark<uint64_t>& benchmark, bool pareto, 
                          const std::vector<int>& params, const std::string& filename) {
  if (!pareto){
    benchmark.template Run<sosd_finedex::FINEdex<uint64_t, Searcher>>(params);
  }
  else {
    benchmark.template Run<sosd_finedex::FINEdex<uint64_t, Searcher>>({32});
    benchmark.template Run<sosd_finedex::FINEdex<uint64_t, Searcher>>({64});
    benchmark.template Run<sosd_finedex::FINEdex<uint64_t, Searcher>>({128});
    benchmark.template Run<sosd_finedex::FINEdex<uint64_t, Searcher>>({256});
    benchmark.template Run<sosd_finedex::FINEdex<uint64_t, Searcher>>({512});
    if (filename.find("books_200M") == std::string::npos) {
      benchmark.template Run<sosd_finedex::FINEdex<uint64_t, Searcher>>({1024});
      benchmark.template Run<sosd_finedex::FINEdex<uint64_t, Searcher>>({2048});
    }
  }
}

template <int record>
void benchmark_64_finedex(sosd::Benchmark<uint64_t>& benchmark, const std::string& filename) {
  if (filename.find("books_200M") != std::string::npos) {
    if (filename.find("0.000000i") != std::string::npos) {
      benchmark.template Run<sosd_finedex::FINEdex<uint64_t, InterpolationSearch<record>>>({256});
      benchmark.template Run<sosd_finedex::FINEdex<uint64_t, InterpolationSearch<record>>>({128});
      benchmark.template Run<sosd_finedex::FINEdex<uint64_t, LinearAVX<uint64_t, record>>>({64});
    } else if (filename.find("mix") == std::string::npos) {
      if (filename.find("0m") != std::string::npos) {
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, LinearAVX<uint64_t, record>>>({64});
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, InterpolationSearch<record>>>({128});
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, BranchingBinarySearch<record>>>({64});
      } else if (filename.find("1m") != std::string::npos) {
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, InterpolationSearch<record>>>({512});
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, LinearSearch<record>>>({256});
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, LinearSearch<record>>>({512});
      } else if (filename.find("2m") != std::string::npos) {
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, LinearSearch<record>>>({64});
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, BranchingBinarySearch<record>>>({128});
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, InterpolationSearch<record>>>({512});
      }
    } else {
      if (filename.find("0.050000i") != std::string::npos) {
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, InterpolationSearch<record>>>({256});
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, InterpolationSearch<record>>>({128});
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, LinearAVX<uint64_t, record>>>({64});
      } else if (filename.find("0.500000i") != std::string::npos) {
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, InterpolationSearch<record>>>({128});
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, LinearAVX<uint64_t, record>>>({64});
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, InterpolationSearch<record>>>({64});
      } else if (filename.find("0.800000i") != std::string::npos) {
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, LinearAVX<uint64_t, record>>>({64});
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, InterpolationSearch<record>>>({128});
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, InterpolationSearch<record>>>({64});
      }
    }
  }
  if (filename.find("fb_200M") != std::string::npos) {
    if (filename.find("0.000000i") != std::string::npos) {
      benchmark.template Run<sosd_finedex::FINEdex<uint64_t, BranchingBinarySearch<record>>>({128});
      benchmark.template Run<sosd_finedex::FINEdex<uint64_t, BranchingBinarySearch<record>>>({64});
      benchmark.template Run<sosd_finedex::FINEdex<uint64_t, BranchingBinarySearch<record>>>({1024});
    } else if (filename.find("mix") == std::string::npos) {
      if (filename.find("0m") != std::string::npos) {
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, BranchingBinarySearch<record>>>({64});
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, BranchingBinarySearch<record>>>({128});
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, LinearAVX<uint64_t, record>>>({64});
      } else if (filename.find("1m") != std::string::npos) {
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, InterpolationSearch<record>>>({512});
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, LinearSearch<record>>>({64});
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, LinearSearch<record>>>({256});
      } else if (filename.find("2m") != std::string::npos) {
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, BranchingBinarySearch<record>>>({64});
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, LinearSearch<record>>>({64});
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, LinearAVX<uint64_t, record>>>({64});
      }
    } else {
      if (filename.find("0.050000i") != std::string::npos) {
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, BranchingBinarySearch<record>>>({128});
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, BranchingBinarySearch<record>>>({64});
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, BranchingBinarySearch<record>>>({1024});
      } else if (filename.find("0.500000i") != std::string::npos) {
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, BranchingBinarySearch<record>>>({64});
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, BranchingBinarySearch<record>>>({128});
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, LinearSearch<record>>>({64});
      } else if (filename.find("0.800000i") != std::string::npos) {
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, BranchingBinarySearch<record>>>({64});
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, BranchingBinarySearch<record>>>({128});
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, LinearSearch<record>>>({64});
      }
    }
  }
  if (filename.find("osm_cellids_200M") != std::string::npos) {
    if (filename.find("0.000000i") != std::string::npos) {
      benchmark.template Run<sosd_finedex::FINEdex<uint64_t, BranchingBinarySearch<record>>>({512});
      benchmark.template Run<sosd_finedex::FINEdex<uint64_t, BranchingBinarySearch<record>>>({1024});
      benchmark.template Run<sosd_finedex::FINEdex<uint64_t, ExponentialSearch<record>>>({128});
    } else if (filename.find("mix") == std::string::npos) {
      if (filename.find("0m") != std::string::npos) {
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, BranchingBinarySearch<record>>>({64});
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, LinearSearch<record>>>({64});
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, LinearAVX<uint64_t, record>>>({64});
      } else if (filename.find("1m") != std::string::npos) {
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, LinearSearch<record>>>({2048});
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, InterpolationSearch<record>>>({32});
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, BranchingBinarySearch<record>>>({32});
      } else if (filename.find("2m") != std::string::npos) {
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, BranchingBinarySearch<record>>>({64});
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, LinearAVX<uint64_t, record>>>({64});
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, BranchingBinarySearch<record>>>({2048});
      }
    } else {
      if (filename.find("0.050000i") != std::string::npos) {
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, BranchingBinarySearch<record>>>({512});
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, BranchingBinarySearch<record>>>({1024});
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, ExponentialSearch<record>>>({128});
      } else if (filename.find("0.500000i") != std::string::npos) {
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, BranchingBinarySearch<record>>>({512});
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, BranchingBinarySearch<record>>>({1024});
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, BranchingBinarySearch<record>>>({64});
      } else if (filename.find("0.800000i") != std::string::npos) {
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, BranchingBinarySearch<record>>>({64});
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, LinearSearch<record>>>({64});
        benchmark.template Run<sosd_finedex::FINEdex<uint64_t, BranchingBinarySearch<record>>>({128});
      }
    }
  }
}

// INSTANTIATE_TEMPLATES_FINEDEX(benchmark_32_finedex, uint32_t);
INSTANTIATE_TEMPLATES_RMI_(benchmark_64_finedex, uint64_t, 0);
INSTANTIATE_TEMPLATES_RMI_(benchmark_64_finedex, uint64_t, 1);
INSTANTIATE_TEMPLATES_RMI_(benchmark_64_finedex, uint64_t, 2);