#include "benchmark.h"
#include "benchmarks/benchmark_mabtree.h"
#include "common.h"
#include "competitors/mabtree.h"

// template <typename Searcher>
// void benchmark_32_mabtree(sosd::Benchmark<uint32_t>& benchmark,
//                          bool pareto) {
//   benchmark.template Run<MABTree<uint32_t, Searcher, 12, 6>>();
//   benchmark.template Run<MABTree<uint32_t, Searcher, 14, 8>>();
//   if (pareto) {
//     benchmark.template Run<MABTree<uint32_t, Searcher, 10, 6>>();
//     benchmark.template Run<MABTree<uint32_t, Searcher, 14, 6>>();
//     benchmark.template Run<MABTree<uint32_t, Searcher, 12, 8>>();
//     benchmark.template Run<MABTree<uint32_t, Searcher, 8, 8>>();
//   }
// }

template <typename Searcher>
void benchmark_64_mabtree(sosd::Benchmark<uint64_t>& benchmark, 
                          bool pareto, const std::vector<int>& params) {
  if (!pareto){
    util::fail("MAB+tree's hyperparameters cannot be set");
  }
  else {
    benchmark.template Run<MABTree<uint64_t, Searcher, 10, 6>>();
    benchmark.template Run<MABTree<uint64_t, Searcher, 10, 7>>();
    benchmark.template Run<MABTree<uint64_t, Searcher, 10, 8>>();
    benchmark.template Run<MABTree<uint64_t, Searcher, 12, 6>>();
    benchmark.template Run<MABTree<uint64_t, Searcher, 12, 7>>();
    benchmark.template Run<MABTree<uint64_t, Searcher, 12, 8>>();
    benchmark.template Run<MABTree<uint64_t, Searcher, 14, 6>>();
    benchmark.template Run<MABTree<uint64_t, Searcher, 14, 7>>();
    benchmark.template Run<MABTree<uint64_t, Searcher, 14, 8>>();
    benchmark.template Run<MABTree<uint64_t, Searcher, 16, 6>>();
    benchmark.template Run<MABTree<uint64_t, Searcher, 16, 7>>();
    benchmark.template Run<MABTree<uint64_t, Searcher, 16, 8>>();
    benchmark.template Run<MABTree<uint64_t, Searcher, 18, 6>>();
    benchmark.template Run<MABTree<uint64_t, Searcher, 18, 7>>();
    benchmark.template Run<MABTree<uint64_t, Searcher, 18, 8>>();
  }
}

template <int record>
void benchmark_64_mabtree(sosd::Benchmark<uint64_t>& benchmark, const std::string& filename) {
  if (filename.find("books_200M") != std::string::npos) {
    if (filename.find("0.000000i") != std::string::npos) {
      benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,12,7>>();
      benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,12,6>>();
      benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,12,8>>();
    } else if (filename.find("mix") == std::string::npos) {
      if (filename.find("0m") != std::string::npos) {
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,10,7>>();
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,10,8>>();
        benchmark.template Run<MABTree<uint64_t, BranchingBinarySearch<record>,10,7>>();
      } else if (filename.find("1m") != std::string::npos) {
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,18,7>>();
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,18,8>>();
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,18,6>>();
      } else if (filename.find("2m") != std::string::npos) {
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,10,7>>();
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,12,6>>();
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,12,8>>();
      }
    } else {
      if (filename.find("0.050000i") != std::string::npos) {
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,12,7>>();
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,12,6>>();
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,12,8>>();
      } else if (filename.find("0.500000i") != std::string::npos) {
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,10,7>>();
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,10,8>>();
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,12,7>>();
      } else if (filename.find("0.800000i") != std::string::npos) {
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,10,7>>();
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,10,8>>();
        benchmark.template Run<MABTree<uint64_t, ExponentialSearch<record>,10,7>>();
      }
    }
  }
  if (filename.find("fb_200M") != std::string::npos) {
    if (filename.find("0.000000i") != std::string::npos) {
      benchmark.template Run<MABTree<uint64_t, BranchingBinarySearch<record>,14,7>>();
      benchmark.template Run<MABTree<uint64_t, BranchingBinarySearch<record>,14,8>>();
      benchmark.template Run<MABTree<uint64_t, BranchingBinarySearch<record>,18,7>>();
    } else if (filename.find("mix") == std::string::npos) {
      if (filename.find("0m") != std::string::npos) {
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,10,7>>();
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,10,8>>();
        benchmark.template Run<MABTree<uint64_t, BranchingBinarySearch<record>,10,7>>();
      } else if (filename.find("1m") != std::string::npos) {
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,18,8>>();
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,18,6>>();
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,18,7>>();
      } else if (filename.find("2m") != std::string::npos) {
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,10,7>>();
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,10,6>>();
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,10,8>>();
      }
    } else {
      if (filename.find("0.050000i") != std::string::npos) {
        benchmark.template Run<MABTree<uint64_t, BranchingBinarySearch<record>,14,7>>();
        benchmark.template Run<MABTree<uint64_t, BranchingBinarySearch<record>,14,8>>();
        benchmark.template Run<MABTree<uint64_t, BranchingBinarySearch<record>,14,6>>();
      } else if (filename.find("0.500000i") != std::string::npos) {
        benchmark.template Run<MABTree<uint64_t, BranchingBinarySearch<record>,10,7>>();
        benchmark.template Run<MABTree<uint64_t, BranchingBinarySearch<record>,10,8>>();
        benchmark.template Run<MABTree<uint64_t, ExponentialSearch<record>,10,7>>();
      } else if (filename.find("0.800000i") != std::string::npos) {
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,10,7>>();
        benchmark.template Run<MABTree<uint64_t, BranchingBinarySearch<record>,10,7>>();
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,10,8>>();
      }
    }
  }
  if (filename.find("osm_cellids_200M") != std::string::npos) {
    if (filename.find("0.000000i") != std::string::npos) {
      benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,10,7>>();
      benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,10,6>>();
      benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,10,8>>();
    } else if (filename.find("mix") == std::string::npos) {
      if (filename.find("0m") != std::string::npos) {
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,10,7>>();
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,10,8>>();
        benchmark.template Run<MABTree<uint64_t, LinearAVX<uint64_t, record>,10,7>>();
      } else if (filename.find("1m") != std::string::npos) {
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,18,7>>();
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,18,8>>();
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,18,6>>();
      } else if (filename.find("2m") != std::string::npos) {
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,10,7>>();
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,10,6>>();
        benchmark.template Run<MABTree<uint64_t, BranchingBinarySearch<record>,10,7>>();
      }
    } else {
      if (filename.find("0.050000i") != std::string::npos) {
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,10,7>>();
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,10,6>>();
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,10,8>>();
      } else if (filename.find("0.500000i") != std::string::npos) {
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,10,7>>();
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,10,8>>();
        benchmark.template Run<MABTree<uint64_t, BranchingBinarySearch<record>,10,7>>();
      } else if (filename.find("0.800000i") != std::string::npos) {
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,10,7>>();
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,10,8>>();
        benchmark.template Run<MABTree<uint64_t, BranchingBinarySearch<record>,10,7>>();
      }
    }
  }
  if (filename.find("wiki_ts_200M") != std::string::npos) {
    if (filename.find("0.000000i") != std::string::npos) {
      benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,12,6>>();
      benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,12,7>>();
      benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,12,8>>();
    } else if (filename.find("mix") == std::string::npos) {
      if (filename.find("0m") != std::string::npos) {
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,10,7>>();
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,10,8>>();
        benchmark.template Run<MABTree<uint64_t, BranchingBinarySearch<record>,10,7>>();
      } else if (filename.find("1m") != std::string::npos) {
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,18,8>>();
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,18,7>>();
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,18,6>>();
      } else if (filename.find("2m") != std::string::npos) {
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,10,7>>();
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,12,6>>();
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,12,8>>();
      }
    } else {
      if (filename.find("0.050000i") != std::string::npos) {
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,12,6>>();
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,12,7>>();
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,10,8>>();
      } else if (filename.find("0.500000i") != std::string::npos) {
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,10,8>>();
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,10,7>>();
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,12,6>>();
      } else if (filename.find("0.800000i") != std::string::npos) {
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,10,7>>();
        benchmark.template Run<MABTree<uint64_t, LinearSearch<record>,10,8>>();
        benchmark.template Run<MABTree<uint64_t, BranchingBinarySearch<record>,10,7>>();
      }
    }
  }
}

// INSTANTIATE_TEMPLATES(benchmark_32_mabtree, uint32_t);
INSTANTIATE_TEMPLATES(benchmark_64_mabtree, uint64_t);
