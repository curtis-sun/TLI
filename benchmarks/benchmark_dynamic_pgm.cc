#include "benchmarks/benchmark_pgm.h"

#include "benchmark.h"
#include "benchmarks/common.h"
#include "competitors/dynamic_pgm_index.h"

template <typename Searcher>
void benchmark_64_dynamic_pgm(tli::Benchmark<uint64_t>& benchmark, 
                              bool pareto, const std::vector<int>& params) {
  if (!pareto){
    util::fail("Dynamic PGM's hyperparameter cannot be set");
  }
  else{
    benchmark.template Run<DynamicPGM<uint64_t, Searcher, 16>>();
    benchmark.template Run<DynamicPGM<uint64_t, Searcher, 32>>();
    benchmark.template Run<DynamicPGM<uint64_t, Searcher, 64>>();
    benchmark.template Run<DynamicPGM<uint64_t, Searcher, 128>>();
    benchmark.template Run<DynamicPGM<uint64_t, Searcher, 256>>();
    benchmark.template Run<DynamicPGM<uint64_t, Searcher, 512>>();
    benchmark.template Run<DynamicPGM<uint64_t, Searcher, 1024>>();
  }
}

template <int record>
void benchmark_64_dynamic_pgm(tli::Benchmark<uint64_t>& benchmark, const std::string& filename) {
  if (filename.find("books_200M") != std::string::npos) {
    if (filename.find("0.000000i") != std::string::npos) {
      benchmark.template Run<DynamicPGM<uint64_t, BranchingBinarySearch<record>,16>>();
      benchmark.template Run<DynamicPGM<uint64_t, LinearSearch<record>,16>>();
      benchmark.template Run<DynamicPGM<uint64_t, BranchingBinarySearch<record>,32>>();
    } else if (filename.find("mix") == std::string::npos) {
      if (filename.find("0m") != std::string::npos) {
        benchmark.template Run<DynamicPGM<uint64_t, InterpolationSearch<record>,256>>();
        benchmark.template Run<DynamicPGM<uint64_t, InterpolationSearch<record>,128>>();
        benchmark.template Run<DynamicPGM<uint64_t, InterpolationSearch<record>,512>>();
      } else if (filename.find("1m") != std::string::npos) {
        benchmark.template Run<DynamicPGM<uint64_t, InterpolationSearch<record>,512>>();
        benchmark.template Run<DynamicPGM<uint64_t, ExponentialSearch<record>,256>>();
        benchmark.template Run<DynamicPGM<uint64_t, ExponentialSearch<record>,512>>();
      } else if (filename.find("2m") != std::string::npos) {
        benchmark.template Run<DynamicPGM<uint64_t, InterpolationSearch<record>,512>>();
        benchmark.template Run<DynamicPGM<uint64_t, ExponentialSearch<record>,512>>();
        benchmark.template Run<DynamicPGM<uint64_t, LinearSearch<record>,256>>();
      }
    } else {
      if (filename.find("0.050000i") != std::string::npos) {
        benchmark.template Run<DynamicPGM<uint64_t, BranchingBinarySearch<record>,16>>();
        benchmark.template Run<DynamicPGM<uint64_t, BranchingBinarySearch<record>,32>>();
        benchmark.template Run<DynamicPGM<uint64_t, LinearSearch<record>,16>>();
      } else if (filename.find("0.500000i") != std::string::npos) {
        benchmark.template Run<DynamicPGM<uint64_t, LinearSearch<record>,32>>();
        benchmark.template Run<DynamicPGM<uint64_t, BranchingBinarySearch<record>,32>>();
        benchmark.template Run<DynamicPGM<uint64_t, BranchingBinarySearch<record>,16>>();
      } else if (filename.find("0.800000i") != std::string::npos) {
        benchmark.template Run<DynamicPGM<uint64_t, LinearSearch<record>,32>>();
        benchmark.template Run<DynamicPGM<uint64_t, InterpolationSearch<record>,128>>();
        benchmark.template Run<DynamicPGM<uint64_t, InterpolationSearch<record>,256>>();
      }
    }
  }
  if (filename.find("fb_200M") != std::string::npos) {
    if (filename.find("0.000000i") != std::string::npos) {
      benchmark.template Run<DynamicPGM<uint64_t, LinearSearch<record>,16>>();
      benchmark.template Run<DynamicPGM<uint64_t, BranchingBinarySearch<record>,128>>();
      benchmark.template Run<DynamicPGM<uint64_t, BranchingBinarySearch<record>,16>>();
    } else if (filename.find("mix") == std::string::npos) {
      if (filename.find("0m") != std::string::npos) {
        benchmark.template Run<DynamicPGM<uint64_t, LinearSearch<record>,512>>();
        benchmark.template Run<DynamicPGM<uint64_t, BranchingBinarySearch<record>,512>>();
        benchmark.template Run<DynamicPGM<uint64_t, ExponentialSearch<record>,256>>();
      } else if (filename.find("1m") != std::string::npos) {
        benchmark.template Run<DynamicPGM<uint64_t, ExponentialSearch<record>,1024>>();
        benchmark.template Run<DynamicPGM<uint64_t, InterpolationSearch<record>,512>>();
        benchmark.template Run<DynamicPGM<uint64_t, BranchingBinarySearch<record>,256>>();
      } else if (filename.find("2m") != std::string::npos) {
        benchmark.template Run<DynamicPGM<uint64_t, BranchingBinarySearch<record>,512>>();
        benchmark.template Run<DynamicPGM<uint64_t, InterpolationSearch<record>,512>>();
        benchmark.template Run<DynamicPGM<uint64_t, LinearSearch<record>,512>>();
      }
    } else {
      if (filename.find("0.050000i") != std::string::npos) {
        benchmark.template Run<DynamicPGM<uint64_t, LinearSearch<record>,16>>();
        benchmark.template Run<DynamicPGM<uint64_t, BranchingBinarySearch<record>,128>>();
        benchmark.template Run<DynamicPGM<uint64_t, BranchingBinarySearch<record>,64>>();
      } else if (filename.find("0.500000i") != std::string::npos) {
        benchmark.template Run<DynamicPGM<uint64_t, BranchingBinarySearch<record>,128>>();
        benchmark.template Run<DynamicPGM<uint64_t, BranchingBinarySearch<record>,64>>();
        benchmark.template Run<DynamicPGM<uint64_t, BranchingBinarySearch<record>,512>>();
      } else if (filename.find("0.800000i") != std::string::npos) {
        benchmark.template Run<DynamicPGM<uint64_t, BranchingBinarySearch<record>,512>>();
        benchmark.template Run<DynamicPGM<uint64_t, BranchingBinarySearch<record>,64>>();
        benchmark.template Run<DynamicPGM<uint64_t, BranchingBinarySearch<record>,128>>();
      }
    }
  }
  if (filename.find("osm_cellids_200M") != std::string::npos) {
    if (filename.find("0.000000i") != std::string::npos) {
      benchmark.template Run<DynamicPGM<uint64_t, BranchingBinarySearch<record>,16>>();
      benchmark.template Run<DynamicPGM<uint64_t, BranchingBinarySearch<record>,32>>();
      benchmark.template Run<DynamicPGM<uint64_t, LinearSearch<record>,16>>();
    } else if (filename.find("mix") == std::string::npos) {
      if (filename.find("0m") != std::string::npos) {
        benchmark.template Run<DynamicPGM<uint64_t, LinearSearch<record>,1024>>();
        benchmark.template Run<DynamicPGM<uint64_t, BranchingBinarySearch<record>,1024>>();
        benchmark.template Run<DynamicPGM<uint64_t, InterpolationSearch<record>,1024>>();
      } else if (filename.find("1m") != std::string::npos) {
        benchmark.template Run<DynamicPGM<uint64_t, BranchingBinarySearch<record>,512>>();
        benchmark.template Run<DynamicPGM<uint64_t, ExponentialSearch<record>,512>>();
        benchmark.template Run<DynamicPGM<uint64_t, BranchingBinarySearch<record>,256>>();
      } else if (filename.find("2m") != std::string::npos) {
        benchmark.template Run<DynamicPGM<uint64_t, LinearSearch<record>,1024>>();
        benchmark.template Run<DynamicPGM<uint64_t, BranchingBinarySearch<record>,1024>>();
        benchmark.template Run<DynamicPGM<uint64_t, InterpolationSearch<record>,1024>>();
      }
    } else {
      if (filename.find("0.050000i") != std::string::npos) {
        benchmark.template Run<DynamicPGM<uint64_t, BranchingBinarySearch<record>,16>>();
        benchmark.template Run<DynamicPGM<uint64_t, BranchingBinarySearch<record>,32>>();
        benchmark.template Run<DynamicPGM<uint64_t, LinearSearch<record>,16>>();
      } else if (filename.find("0.500000i") != std::string::npos) {
        benchmark.template Run<DynamicPGM<uint64_t, BranchingBinarySearch<record>,32>>();
        benchmark.template Run<DynamicPGM<uint64_t, BranchingBinarySearch<record>,64>>();
        benchmark.template Run<DynamicPGM<uint64_t, BranchingBinarySearch<record>,128>>();
      } else if (filename.find("0.800000i") != std::string::npos) {
        benchmark.template Run<DynamicPGM<uint64_t, BranchingBinarySearch<record>,256>>();
        benchmark.template Run<DynamicPGM<uint64_t, BranchingBinarySearch<record>,64>>();
        benchmark.template Run<DynamicPGM<uint64_t, BranchingBinarySearch<record>,128>>();
      }
    }
  }
  if (filename.find("wiki_ts_200M") != std::string::npos) {
    if (filename.find("0.000000i") != std::string::npos) {
      benchmark.template Run<DynamicPGM<uint64_t, LinearSearch<record>,16>>();
      benchmark.template Run<DynamicPGM<uint64_t, LinearSearch<record>,32>>();
      benchmark.template Run<DynamicPGM<uint64_t, BranchingBinarySearch<record>,16>>();
    } else if (filename.find("mix") == std::string::npos) {
      if (filename.find("0m") != std::string::npos) {
        benchmark.template Run<DynamicPGM<uint64_t, ExponentialSearch<record>,512>>();
        benchmark.template Run<DynamicPGM<uint64_t, InterpolationSearch<record>,512>>();
        benchmark.template Run<DynamicPGM<uint64_t, InterpolationSearch<record>,128>>();
      } else if (filename.find("1m") != std::string::npos) {
        benchmark.template Run<DynamicPGM<uint64_t, InterpolationSearch<record>,512>>();
        benchmark.template Run<DynamicPGM<uint64_t, BranchingBinarySearch<record>,512>>();
        benchmark.template Run<DynamicPGM<uint64_t, ExponentialSearch<record>,512>>();
      } else if (filename.find("2m") != std::string::npos) {
        benchmark.template Run<DynamicPGM<uint64_t, LinearSearch<record>,32>>();
        benchmark.template Run<DynamicPGM<uint64_t, BranchingBinarySearch<record>,32>>();
        benchmark.template Run<DynamicPGM<uint64_t, InterpolationSearch<record>,32>>();
      }
    } else {
      if (filename.find("0.050000i") != std::string::npos) {
        benchmark.template Run<DynamicPGM<uint64_t, LinearSearch<record>,32>>();
        benchmark.template Run<DynamicPGM<uint64_t, LinearSearch<record>,16>>();
        benchmark.template Run<DynamicPGM<uint64_t, BranchingBinarySearch<record>,16>>();
      } else if (filename.find("0.500000i") != std::string::npos) {
        benchmark.template Run<DynamicPGM<uint64_t, LinearSearch<record>,32>>();
        benchmark.template Run<DynamicPGM<uint64_t, BranchingBinarySearch<record>,32>>();
        benchmark.template Run<DynamicPGM<uint64_t, InterpolationSearch<record>,128>>();
      } else if (filename.find("0.800000i") != std::string::npos) {
        benchmark.template Run<DynamicPGM<uint64_t, InterpolationSearch<record>,128>>();
        benchmark.template Run<DynamicPGM<uint64_t, InterpolationSearch<record>,512>>();
        benchmark.template Run<DynamicPGM<uint64_t, BranchingBinarySearch<record>,128>>();
      }
    }
  }
}

INSTANTIATE_TEMPLATES(benchmark_64_dynamic_pgm, uint64_t);