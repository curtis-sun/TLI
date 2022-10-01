#include "benchmarks/benchmark_alex.h"

#include "benchmark.h"
#include "common.h"
#include "competitors/alex.h"

// template <typename Searcher>
// void benchmark_32_alex(sosd::Benchmark<uint32_t>& benchmark,
//                        bool pareto) {
//   benchmark.template Run<Alex<uint32_t, Searcher, 28>>();
//   if (pareto) {
//     benchmark.template Run<Alex<uint32_t, Searcher, 16>>();
//     benchmark.template Run<Alex<uint32_t, Searcher, 20>>();
//     benchmark.template Run<Alex<uint32_t, Searcher, 26>>();
//     benchmark.template Run<Alex<uint32_t, Searcher, 30>>();
//   }
// }

template <typename Searcher>
void benchmark_64_alex(sosd::Benchmark<uint64_t>& benchmark, 
                       bool pareto, const std::vector<int>& params) {
  if (!pareto){
    benchmark.template Run<Alex<uint64_t, Searcher>>(params);
  }
  else {
    benchmark.template Run<Alex<uint64_t, Searcher>>({14});
    benchmark.template Run<Alex<uint64_t, Searcher>>({16});
    benchmark.template Run<Alex<uint64_t, Searcher>>({18});
    benchmark.template Run<Alex<uint64_t, Searcher>>({20});
    benchmark.template Run<Alex<uint64_t, Searcher>>({22});
    benchmark.template Run<Alex<uint64_t, Searcher>>({24});
    benchmark.template Run<Alex<uint64_t, Searcher>>({26});
  }
}

template <int record>
void benchmark_64_alex(sosd::Benchmark<uint64_t>& benchmark, const std::string& filename) {
  if (filename.find("books_200M") != std::string::npos) {
    if (filename.find("0.000000i") != std::string::npos) {
      benchmark.template Run<Alex<uint64_t, LinearSearch<record>>>({26});
      benchmark.template Run<Alex<uint64_t, LinearSearch<record>>>({24});
      benchmark.template Run<Alex<uint64_t, LinearSearch<record>>>({22});
    } 
    else if (filename.find("mix") == std::string::npos) {
      if (filename.find("0m") != std::string::npos) {
        benchmark.template Run<Alex<uint64_t, LinearSearch<record>>>({22});
        benchmark.template Run<Alex<uint64_t, LinearSearch<record>>>({20});
        benchmark.template Run<Alex<uint64_t, LinearSearch<record>>>({24});
      } else if (filename.find("1m") != std::string::npos) {
        benchmark.template Run<Alex<uint64_t, BranchingBinarySearch<record>>>({26});
        benchmark.template Run<Alex<uint64_t, BranchingBinarySearch<record>>>({16});
        benchmark.template Run<Alex<uint64_t, BranchingBinarySearch<record>>>({18});
      } else if (filename.find("2m") != std::string::npos) {
        benchmark.template Run<Alex<uint64_t, LinearSearch<record>>>({26});
        benchmark.template Run<Alex<uint64_t, LinearSearch<record>>>({22});
        benchmark.template Run<Alex<uint64_t, LinearSearch<record>>>({20});
      }
    } else {
      if (filename.find("0.050000i") != std::string::npos) {
        benchmark.template Run<Alex<uint64_t, LinearSearch<record>>>({26});
        benchmark.template Run<Alex<uint64_t, LinearSearch<record>>>({24});
        benchmark.template Run<Alex<uint64_t, LinearSearch<record>>>({22});
      } else if (filename.find("0.500000i") != std::string::npos) {
        benchmark.template Run<Alex<uint64_t, LinearSearch<record>>>({26});
        benchmark.template Run<Alex<uint64_t, LinearSearch<record>>>({24});
        benchmark.template Run<Alex<uint64_t, LinearSearch<record>>>({22});
      } else if (filename.find("0.800000i") != std::string::npos) {
        benchmark.template Run<Alex<uint64_t, LinearSearch<record>>>({22});
        benchmark.template Run<Alex<uint64_t, LinearSearch<record>>>({20});
        benchmark.template Run<Alex<uint64_t, LinearSearch<record>>>({24});
      }
    }
  }
  if (filename.find("fb_200M") != std::string::npos) {
    if (filename.find("0.000000i") != std::string::npos) {
      benchmark.template Run<Alex<uint64_t, BranchingBinarySearch<record>>>({22});
      benchmark.template Run<Alex<uint64_t, BranchingBinarySearch<record>>>({26});
      benchmark.template Run<Alex<uint64_t, ExponentialSearch<record>>>({22});
    } else if (filename.find("mix") == std::string::npos) {
      if (filename.find("0m") != std::string::npos) {
        benchmark.template Run<Alex<uint64_t, LinearAVX<uint64_t, record>>>({14});
        benchmark.template Run<Alex<uint64_t, BranchingBinarySearch<record>>>({14});
        benchmark.template Run<Alex<uint64_t, ExponentialSearch<record>>>({14});
      } else if (filename.find("1m") != std::string::npos) {
        benchmark.template Run<Alex<uint64_t, BranchingBinarySearch<record>>>({26});
        benchmark.template Run<Alex<uint64_t, BranchingBinarySearch<record>>>({18});
        benchmark.template Run<Alex<uint64_t, BranchingBinarySearch<record>>>({16});
      } else if (filename.find("2m") != std::string::npos) {
        benchmark.template Run<Alex<uint64_t, LinearAVX<uint64_t, record>>>({16});
        benchmark.template Run<Alex<uint64_t, LinearAVX<uint64_t, record>>>({20});
        benchmark.template Run<Alex<uint64_t, LinearAVX<uint64_t, record>>>({14});
      }
    } else {
      if (filename.find("0.050000i") != std::string::npos) {
        benchmark.template Run<Alex<uint64_t, BranchingBinarySearch<record>>>({22});
        benchmark.template Run<Alex<uint64_t, BranchingBinarySearch<record>>>({26});
        benchmark.template Run<Alex<uint64_t, ExponentialSearch<record>>>({22});
      } else if (filename.find("0.500000i") != std::string::npos) {
        benchmark.template Run<Alex<uint64_t, ExponentialSearch<record>>>({14});
        benchmark.template Run<Alex<uint64_t, BranchingBinarySearch<record>>>({14});
        benchmark.template Run<Alex<uint64_t, LinearSearch<record>>>({14});
      } else if (filename.find("0.800000i") != std::string::npos) {
        benchmark.template Run<Alex<uint64_t, BranchingBinarySearch<record>>>({14});
        benchmark.template Run<Alex<uint64_t, ExponentialSearch<record>>>({14});
        benchmark.template Run<Alex<uint64_t, LinearAVX<uint64_t, record>>>({14});
      }
    }
  }
  if (filename.find("osm_cellids_200M") != std::string::npos) {
    if (filename.find("0.000000i") != std::string::npos) {
      benchmark.template Run<Alex<uint64_t, ExponentialSearch<record>>>({18});
      benchmark.template Run<Alex<uint64_t, ExponentialSearch<record>>>({20});
      benchmark.template Run<Alex<uint64_t, ExponentialSearch<record>>>({16});
    } else if (filename.find("mix") == std::string::npos) {
      if (filename.find("0m") != std::string::npos) {
        benchmark.template Run<Alex<uint64_t, LinearSearch<record>>>({14});
        benchmark.template Run<Alex<uint64_t, LinearAVX<uint64_t, record>>>({14});
        benchmark.template Run<Alex<uint64_t, BranchingBinarySearch<record>>>({14});
      } else if (filename.find("1m") != std::string::npos) {
        benchmark.template Run<Alex<uint64_t, BranchingBinarySearch<record>>>({24});
        benchmark.template Run<Alex<uint64_t, BranchingBinarySearch<record>>>({26});
        benchmark.template Run<Alex<uint64_t, BranchingBinarySearch<record>>>({22});
      } else if (filename.find("2m") != std::string::npos) {
        benchmark.template Run<Alex<uint64_t, LinearAVX<uint64_t, record>>>({24});
        benchmark.template Run<Alex<uint64_t, LinearAVX<uint64_t, record>>>({18});
        benchmark.template Run<Alex<uint64_t, LinearAVX<uint64_t, record>>>({16});
      }
    } else {
      if (filename.find("0.050000i") != std::string::npos) {
        benchmark.template Run<Alex<uint64_t, ExponentialSearch<record>>>({18});
        benchmark.template Run<Alex<uint64_t, ExponentialSearch<record>>>({20});
        benchmark.template Run<Alex<uint64_t, ExponentialSearch<record>>>({16});
      } else if (filename.find("0.500000i") != std::string::npos) {
        benchmark.template Run<Alex<uint64_t, LinearSearch<record>>>({14});
        benchmark.template Run<Alex<uint64_t, ExponentialSearch<record>>>({14});
        benchmark.template Run<Alex<uint64_t, LinearAVX<uint64_t, record>>>({14});
      } else if (filename.find("0.800000i") != std::string::npos) {
        benchmark.template Run<Alex<uint64_t, LinearSearch<record>>>({14});
        benchmark.template Run<Alex<uint64_t, LinearAVX<uint64_t, record>>>({14});
        benchmark.template Run<Alex<uint64_t, ExponentialSearch<record>>>({14});
      }
    }
  }
  if (filename.find("wiki_ts_200M") != std::string::npos) {
    if (filename.find("0.000000i") != std::string::npos) {
      benchmark.template Run<Alex<uint64_t, LinearSearch<record>>>({20});
      benchmark.template Run<Alex<uint64_t, LinearSearch<record>>>({22});
      benchmark.template Run<Alex<uint64_t, LinearSearch<record>>>({26});
    } else if (filename.find("mix") == std::string::npos) {
      if (filename.find("0m") != std::string::npos) {
        benchmark.template Run<Alex<uint64_t, LinearSearch<record>>>({20});
        benchmark.template Run<Alex<uint64_t, LinearSearch<record>>>({22});
        benchmark.template Run<Alex<uint64_t, LinearAVX<uint64_t, record>>>({20});
      } else if (filename.find("1m") != std::string::npos) {
        benchmark.template Run<Alex<uint64_t, LinearSearch<record>>>({24});
        benchmark.template Run<Alex<uint64_t, LinearSearch<record>>>({22});
        benchmark.template Run<Alex<uint64_t, LinearSearch<record>>>({20});
      } else if (filename.find("2m") != std::string::npos) {
        benchmark.template Run<Alex<uint64_t, LinearSearch<record>>>({22});
        benchmark.template Run<Alex<uint64_t, LinearSearch<record>>>({26});
        benchmark.template Run<Alex<uint64_t, LinearSearch<record>>>({24});
      }
    } else {
      if (filename.find("0.050000i") != std::string::npos) {
        benchmark.template Run<Alex<uint64_t, LinearSearch<record>>>({20});
        benchmark.template Run<Alex<uint64_t, LinearSearch<record>>>({22});
        benchmark.template Run<Alex<uint64_t, LinearSearch<record>>>({26});
      } else if (filename.find("0.500000i") != std::string::npos) {
        benchmark.template Run<Alex<uint64_t, LinearSearch<record>>>({20});
        benchmark.template Run<Alex<uint64_t, LinearSearch<record>>>({22});
        benchmark.template Run<Alex<uint64_t, LinearSearch<record>>>({24});
      } else if (filename.find("0.800000i") != std::string::npos) {
        benchmark.template Run<Alex<uint64_t, LinearSearch<record>>>({20});
        benchmark.template Run<Alex<uint64_t, LinearSearch<record>>>({22});
        benchmark.template Run<Alex<uint64_t, LinearAVX<uint64_t, record>>>({20});
      }
    }
  }
}

// INSTANTIATE_TEMPLATES(benchmark_32_alex, uint32_t);
INSTANTIATE_TEMPLATES(benchmark_64_alex, uint64_t);