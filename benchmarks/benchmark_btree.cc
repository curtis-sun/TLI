#include "benchmarks/benchmark_btree.h"

#include "benchmark.h"
#include "common.h"
#include "competitors/stx_btree.h"

// template <typename Searcher>
// void benchmark_32_btree(sosd::Benchmark<uint32_t>& benchmark,
//                        bool pareto) {
//   benchmark.template Run<STXBTree<uint32_t, Searcher, 10>>();
//   if (pareto) {
//     benchmark.template Run<STXBTree<uint32_t, Searcher, 6>>();
//     benchmark.template Run<STXBTree<uint32_t, Searcher, 8>>();
//     benchmark.template Run<STXBTree<uint32_t, Searcher, 14>>();
//     benchmark.template Run<STXBTree<uint32_t, Searcher, 16>>();
//   }
// }

template <typename Searcher>
void benchmark_64_btree(sosd::Benchmark<uint64_t>& benchmark, 
                        bool pareto, const std::vector<int>& params) {
  if (!pareto){
    util::fail("B+tree's hyperparameter cannot be set");
  }
  else{
    benchmark.template Run<STXBTree<uint64_t, Searcher, 6>>();
    benchmark.template Run<STXBTree<uint64_t, Searcher, 8>>();
    benchmark.template Run<STXBTree<uint64_t, Searcher, 10>>();
    benchmark.template Run<STXBTree<uint64_t, Searcher, 12>>();
    benchmark.template Run<STXBTree<uint64_t, Searcher, 14>>();
    benchmark.template Run<STXBTree<uint64_t, Searcher, 16>>();
    benchmark.template Run<STXBTree<uint64_t, Searcher, 18>>();
  }
}

template <int record>
void benchmark_64_btree(sosd::Benchmark<uint64_t>& benchmark, const std::string& filename) {
  if (filename.find("books_200M") != std::string::npos) {
    if (filename.find("0.000000i") != std::string::npos) {
      benchmark.template Run<STXBTree<uint64_t, LinearSearch<record>,6>>();
      benchmark.template Run<STXBTree<uint64_t, LinearSearch<record>,8>>();
      benchmark.template Run<STXBTree<uint64_t, InterpolationSearch<record>,16>>();
    } else if (filename.find("mix") == std::string::npos) {
      if (filename.find("0m") != std::string::npos) {
        benchmark.template Run<STXBTree<uint64_t, InterpolationSearch<record>,10>>();
        benchmark.template Run<STXBTree<uint64_t, LinearSearch<record>,8>>();
        benchmark.template Run<STXBTree<uint64_t, InterpolationSearch<record>,12>>();
      } else if (filename.find("1m") != std::string::npos) {
        benchmark.template Run<STXBTree<uint64_t, InterpolationSearch<record>,18>>();
        benchmark.template Run<STXBTree<uint64_t, InterpolationSearch<record>,16>>();
        benchmark.template Run<STXBTree<uint64_t, InterpolationSearch<record>,14>>();
      } else if (filename.find("2m") != std::string::npos) {
        benchmark.template Run<STXBTree<uint64_t, LinearSearch<record>,8>>();
        benchmark.template Run<STXBTree<uint64_t, InterpolationSearch<record>,8>>();
        benchmark.template Run<STXBTree<uint64_t, LinearAVX<uint64_t, record>,10>>();
      }
    } else {
      if (filename.find("0.050000i") != std::string::npos) {
        benchmark.template Run<STXBTree<uint64_t, LinearSearch<record>,6>>();
        benchmark.template Run<STXBTree<uint64_t, LinearSearch<record>,8>>();
        benchmark.template Run<STXBTree<uint64_t, InterpolationSearch<record>,16>>();
      } else if (filename.find("0.500000i") != std::string::npos) {
        benchmark.template Run<STXBTree<uint64_t, LinearSearch<record>,8>>();
        benchmark.template Run<STXBTree<uint64_t, LinearSearch<record>,6>>();
        benchmark.template Run<STXBTree<uint64_t, InterpolationSearch<record>,12>>();
      } else if (filename.find("0.800000i") != std::string::npos) {
        benchmark.template Run<STXBTree<uint64_t, LinearSearch<record>,8>>();
        benchmark.template Run<STXBTree<uint64_t, InterpolationSearch<record>,10>>();
        benchmark.template Run<STXBTree<uint64_t, InterpolationSearch<record>,12>>();
      }
    }
  }
  if (filename.find("fb_200M") != std::string::npos) {
    if (filename.find("0.000000i") != std::string::npos) {
      benchmark.template Run<STXBTree<uint64_t, LinearSearch<record>,6>>();
      benchmark.template Run<STXBTree<uint64_t, LinearSearch<record>,8>>();
      benchmark.template Run<STXBTree<uint64_t, InterpolationSearch<record>,6>>();
    } else if (filename.find("mix") == std::string::npos) {
      if (filename.find("0m") != std::string::npos) {
        benchmark.template Run<STXBTree<uint64_t, InterpolationSearch<record>,10>>();
        benchmark.template Run<STXBTree<uint64_t, InterpolationSearch<record>,12>>();
        benchmark.template Run<STXBTree<uint64_t, LinearSearch<record>,8>>();
      } else if (filename.find("1m") != std::string::npos) {
        benchmark.template Run<STXBTree<uint64_t, InterpolationSearch<record>,18>>();
        benchmark.template Run<STXBTree<uint64_t, InterpolationSearch<record>,16>>();
        benchmark.template Run<STXBTree<uint64_t, InterpolationSearch<record>,14>>();
      } else if (filename.find("2m") != std::string::npos) {
        benchmark.template Run<STXBTree<uint64_t, LinearSearch<record>,6>>();
        benchmark.template Run<STXBTree<uint64_t, LinearSearch<record>,8>>();
        benchmark.template Run<STXBTree<uint64_t, LinearAVX<uint64_t, record>,10>>();
      }
    } else {
      if (filename.find("0.050000i") != std::string::npos) {
        benchmark.template Run<STXBTree<uint64_t, LinearSearch<record>,6>>();
        benchmark.template Run<STXBTree<uint64_t, LinearSearch<record>,8>>();
        benchmark.template Run<STXBTree<uint64_t, InterpolationSearch<record>,6>>();
      } else if (filename.find("0.500000i") != std::string::npos) {
        benchmark.template Run<STXBTree<uint64_t, LinearSearch<record>,8>>();
        benchmark.template Run<STXBTree<uint64_t, LinearSearch<record>,6>>();
        benchmark.template Run<STXBTree<uint64_t, InterpolationSearch<record>,10>>();
      } else if (filename.find("0.800000i") != std::string::npos) {
        benchmark.template Run<STXBTree<uint64_t, LinearSearch<record>,8>>();
        benchmark.template Run<STXBTree<uint64_t, InterpolationSearch<record>,10>>();
        benchmark.template Run<STXBTree<uint64_t, LinearSearch<record>,6>>();
      }
    }
  }
  if (filename.find("osm_cellids_200M") != std::string::npos) {
    if (filename.find("0.000000i") != std::string::npos) {
      benchmark.template Run<STXBTree<uint64_t, LinearSearch<record>,6>>();
      benchmark.template Run<STXBTree<uint64_t, LinearSearch<record>,8>>();
      benchmark.template Run<STXBTree<uint64_t, BranchingBinarySearch<record>,18>>();
    } else if (filename.find("mix") == std::string::npos) {
      if (filename.find("0m") != std::string::npos) {
        benchmark.template Run<STXBTree<uint64_t, LinearSearch<record>,8>>();
        benchmark.template Run<STXBTree<uint64_t, LinearSearch<record>,6>>();
        benchmark.template Run<STXBTree<uint64_t, LinearAVX<uint64_t, record>,10>>();
      } else if (filename.find("1m") != std::string::npos) {
        benchmark.template Run<STXBTree<uint64_t, InterpolationSearch<record>,18>>();
        benchmark.template Run<STXBTree<uint64_t, InterpolationSearch<record>,16>>();
        benchmark.template Run<STXBTree<uint64_t, InterpolationSearch<record>,14>>();
      } else if (filename.find("2m") != std::string::npos) {
        benchmark.template Run<STXBTree<uint64_t, LinearSearch<record>,6>>();
        benchmark.template Run<STXBTree<uint64_t, LinearSearch<record>,8>>();
        benchmark.template Run<STXBTree<uint64_t, LinearAVX<uint64_t, record>,10>>();
      }
    } else {
      if (filename.find("0.050000i") != std::string::npos) {
        benchmark.template Run<STXBTree<uint64_t, LinearSearch<record>,6>>();
        benchmark.template Run<STXBTree<uint64_t, LinearSearch<record>,8>>();
        benchmark.template Run<STXBTree<uint64_t, BranchingBinarySearch<record>,18>>();
      } else if (filename.find("0.500000i") != std::string::npos) {
        benchmark.template Run<STXBTree<uint64_t, LinearSearch<record>,8>>();
        benchmark.template Run<STXBTree<uint64_t, LinearSearch<record>,6>>();
        benchmark.template Run<STXBTree<uint64_t, BranchingBinarySearch<record>,10>>();
      } else if (filename.find("0.800000i") != std::string::npos) {
        benchmark.template Run<STXBTree<uint64_t, LinearSearch<record>,8>>();
        benchmark.template Run<STXBTree<uint64_t, LinearSearch<record>,6>>();
        benchmark.template Run<STXBTree<uint64_t, LinearAVX<uint64_t, record>,10>>();
      }
    }
  }
  if (filename.find("wiki_ts_200M") != std::string::npos) {
    if (filename.find("0.000000i") != std::string::npos) {
      benchmark.template Run<STXBTree<uint64_t, LinearSearch<record>,8>>();
      benchmark.template Run<STXBTree<uint64_t, LinearSearch<record>,6>>();
      benchmark.template Run<STXBTree<uint64_t, InterpolationSearch<record>,14>>();
    } else if (filename.find("mix") == std::string::npos) {
      if (filename.find("0m") != std::string::npos) {
        benchmark.template Run<STXBTree<uint64_t, InterpolationSearch<record>,10>>();
        benchmark.template Run<STXBTree<uint64_t, LinearSearch<record>,8>>();
        benchmark.template Run<STXBTree<uint64_t, InterpolationSearch<record>,12>>();
      } else if (filename.find("1m") != std::string::npos) {
        benchmark.template Run<STXBTree<uint64_t, InterpolationSearch<record>,18>>();
        benchmark.template Run<STXBTree<uint64_t, InterpolationSearch<record>,16>>();
        benchmark.template Run<STXBTree<uint64_t, InterpolationSearch<record>,12>>();
      } else if (filename.find("2m") != std::string::npos) {
        benchmark.template Run<STXBTree<uint64_t, LinearSearch<record>,6>>();
        benchmark.template Run<STXBTree<uint64_t, LinearSearch<record>,8>>();
        benchmark.template Run<STXBTree<uint64_t, InterpolationSearch<record>,8>>();
      }
    } else {
      if (filename.find("0.050000i") != std::string::npos) {
        benchmark.template Run<STXBTree<uint64_t, LinearSearch<record>,8>>();
        benchmark.template Run<STXBTree<uint64_t, LinearSearch<record>,6>>();
        benchmark.template Run<STXBTree<uint64_t, InterpolationSearch<record>,12>>();
      } else if (filename.find("0.500000i") != std::string::npos) {
        benchmark.template Run<STXBTree<uint64_t, LinearSearch<record>,8>>();
        benchmark.template Run<STXBTree<uint64_t, LinearSearch<record>,6>>();
        benchmark.template Run<STXBTree<uint64_t, InterpolationSearch<record>,10>>();
      } else if (filename.find("0.800000i") != std::string::npos) {
        benchmark.template Run<STXBTree<uint64_t, LinearSearch<record>,8>>();
        benchmark.template Run<STXBTree<uint64_t, InterpolationSearch<record>,10>>();
        benchmark.template Run<STXBTree<uint64_t, InterpolationSearch<record>,12>>();
      }
    }
  }
}

// INSTANTIATE_TEMPLATES(benchmark_32_btree, uint32_t);
INSTANTIATE_TEMPLATES(benchmark_64_btree, uint64_t);
