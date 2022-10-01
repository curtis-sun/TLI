#include "benchmarks/benchmark_rmi.h"

#include <string>

#include "benchmark.h"
#include "benchmarks/common.h"
#include "competitors/rmi.h"

#define NAME2(a, b) NAME2_HIDDEN(a, b)
#define NAME2_HIDDEN(a, b) a##b

#define NAME3(a, b, c) NAME3_HIDDEN(a, b, c)
#define NAME3_HIDDEN(a, b, c) a##b##c

#define NAME5(a, b, c, d, e) NAME5_HIDDEN(a, b, c, d, e)
#define NAME5_HIDDEN(a, b, c, d, e) a##b##c##d##e

#define run_rmi(dtype, name, search_class, suffix, variant)                    \
  if (filename.find(#name "_" #dtype) != std::string::npos) {                  \
      benchmark                                                                \
          .template Run<RMI_B<NAME2(dtype, _t), search_class, variant,         \
                              NAME5(name, _, dtype, _, suffix)::BUILD_TIME_NS, \
                              NAME5(name, _, dtype, _, suffix)::RMI_SIZE,      \
                              NAME5(name, _, dtype, _, suffix)::NAME,          \
                              NAME5(name, _, dtype, _, suffix)::lookup,        \
                              NAME5(name, _, dtype, _, suffix)::load,          \
                              NAME5(name, _, dtype, _, suffix)::cleanup>>();   \
    }   

#define run_rmi_pareto(dtype, name, search_class)                              \
  {                                                                            \
    run_rmi(dtype, name, search_class, 0, 0);                                  \
    run_rmi(dtype, name, search_class, 1, 1);                                  \
    run_rmi(dtype, name, search_class, 2, 2);                                  \
    run_rmi(dtype, name, search_class, 3, 3);                                  \
    run_rmi(dtype, name, search_class, 4, 4);                                  \
    run_rmi(dtype, name, search_class, 5, 5);                                  \
    run_rmi(dtype, name, search_class, 6, 6);                                  \
  }

// #define run_rmi_pareto_size(dtype, name, search_class)                       
//   {                                                                          
//     run_rmi_pareto(dtype, NAME3(name, _, 200M), search_class)                
//     run_rmi_pareto(dtype, NAME3(name, _, 100M), search_class)                
//     run_rmi_pareto(dtype, NAME3(name, _, 50M), search_class)                 
//     run_rmi_pareto(dtype, NAME3(name, _, 25M), search_class)                 
//     run_rmi_pareto(dtype, NAME3(name, _, 12500K), search_class)              
//   }

// template <typename Searcher>
// void benchmark_32_rmi(sosd::Benchmark<uint32_t>& benchmark,
//                       bool pareto, const std::string& filename) {
//   run_rmi_pareto(uint32, books_100M, Searcher);
//   run_rmi_pareto(uint32, books_50M, Searcher);
//   run_rmi_pareto(uint32, books_25M, Searcher);
//   run_rmi_pareto(uint32, books_12500K, Searcher);
  
//   run_rmi_pareto(uint32, uniform_dense_200M, Searcher);
//   run_rmi_pareto(uint32, uniform_sparse_200M, Searcher);
//   run_rmi_pareto(uint32, normal_200M, Searcher);
//   run_rmi_pareto(uint32, lognormal_200M, Searcher);
// }

template <typename Searcher>
void benchmark_64_rmi(sosd::Benchmark<uint64_t>& benchmark, 
                      bool pareto, const std::vector<int>& params, const std::string& filename) {
  if (!pareto){
    util::fail("RMI's hyperparameters cannot be set");
  }
  else {
    run_rmi_pareto(uint64, fb_200M, Searcher);
    run_rmi_pareto(uint64, wiki_ts_200M, Searcher);
    run_rmi_pareto(uint64, osm_cellids_200M, Searcher);
    run_rmi_pareto(uint64, books_200M, Searcher);
    // run_rmi_pareto_size(uint64, fb, Searcher);
    // run_rmi_pareto_size(uint64, wiki_ts, Searcher);
    // run_rmi_pareto_size(uint64, osm_cellids, Searcher);
    // run_rmi_pareto_size(uint64, books, Searcher);

    // run_rmi_pareto(uint64, osm_cellids_800M, Searcher);
    // run_rmi_pareto(uint64, books_800M, Searcher);

    // run_rmi_pareto(uint64, uniform_dense_200M, Searcher);
    // run_rmi_pareto(uint64, uniform_sparse_200M, Searcher);
    // run_rmi_pareto(uint64, lognormal_200M, Searcher);
    // run_rmi_pareto(uint64, normal_200M, Searcher);
  }
}

template <int record>
void benchmark_64_rmi(sosd::Benchmark<uint64_t>& benchmark, const std::string& filename) {
  if (filename.find("bulkload") == std::string::npos) {
    if (filename.find("books_200M") != std::string::npos) {
      run_rmi(uint64, books_200M, LinearSearch<record>, 0, 0);
      run_rmi(uint64, books_200M, LinearSearch<record>, 1, 1);
      run_rmi(uint64, books_200M, LinearSearch<record>, 2, 2);
    }
    if (filename.find("fb_200M") != std::string::npos) {
      run_rmi(uint64, fb_200M, LinearSearch<record>, 1, 1);
      run_rmi(uint64, fb_200M, LinearSearch<record>, 0, 0);
      run_rmi(uint64, fb_200M, LinearSearch<record>, 2, 2);
    }
    if (filename.find("osm_cellids_200M") != std::string::npos) {
      run_rmi(uint64, osm_cellids_200M, BranchingBinarySearch<record>, 1, 1);
      run_rmi(uint64, osm_cellids_200M, BranchingBinarySearch<record>, 0, 0);
      run_rmi(uint64, osm_cellids_200M, BranchingBinarySearch<record>, 2, 2);
    }
    if (filename.find("wiki_ts_200M") != std::string::npos) {
      run_rmi(uint64, wiki_ts_200M, LinearSearch<record>, 1, 1);
      run_rmi(uint64, wiki_ts_200M, LinearSearch<record>, 2, 2);
      run_rmi(uint64, wiki_ts_200M, LinearSearch<record>, 0, 0);
    }
  }
}

// INSTANTIATE_TEMPLATES_RMI(benchmark_32_rmi, uint32_t);
INSTANTIATE_TEMPLATES_RMI_(benchmark_64_rmi, uint64_t, 0);
INSTANTIATE_TEMPLATES_RMI_(benchmark_64_rmi, uint64_t, 1);
