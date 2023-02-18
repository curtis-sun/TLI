#include "benchmark.h"

#include <cstdlib>

#include "benchmarks/benchmark_alex.h"
#include "benchmarks/benchmark_lipp.h"
#include "benchmarks/benchmark_art.h"
#include "benchmarks/benchmark_artolc.h"
#include "benchmarks/benchmark_btree.h"
#include "benchmarks/benchmark_xindex.h"
#include "benchmarks/benchmark_finedex.h"
#include "benchmarks/benchmark_sindex.h"
#include "benchmarks/benchmark_fast.h"
#include "benchmarks/benchmark_fst.h"
#include "benchmarks/benchmark_mabtree.h"
#include "benchmarks/benchmark_pgm.h"
#include "benchmarks/benchmark_dynamic_pgm.h"
#include "benchmarks/benchmark_rmi.h"
#include "benchmarks/benchmark_ts.h"
#include "benchmarks/benchmark_wormhole.h"
#include "config.h"
#include "searches/linear_search.h"
#include "searches/linear_search_avx.h"
#include "searches/branching_binary_search.h"
#include "searches/exponential_search.h"
#include "searches/interpolation_search.h"
#include "util.h"
#include "utils/cxxopts.hpp"
using namespace std;

#define COMMA ,

#define check_only(tag, code)                                                                             \
  if (!only_mode || (only == (tag))) {                                                                    \
    code;                                                                                                 \
  }
#define add_search_type(name, func, type, search_class, record)                                           \
  if (search_type == (name) ) {                                                                           \
    sosd::Benchmark<type> benchmark(                                                                      \
        filename, ops, num_repeats, through, build, fence, cold_cache,                                    \
        track_errors, csv, num_threads, verify);                                                          \
    func<search_class, record>(benchmark, pareto, params, only_mode, only, filename);                     \
    break;                                                                                                \
  }
#define add_default(func, type, record)                                                                   \
  if (!pareto && params.empty()) {                                                                        \
    sosd::Benchmark<type> benchmark(                                                                      \
        filename, ops, num_repeats, through, build, fence, cold_cache,                                    \
        track_errors, csv, num_threads, verify);                                                          \
    func<record>(benchmark, only_mode, only, ops);                                                        \
    break;                                                                                                \
  }
#define add_search_types(func, type, record)                                                              \
  add_default(func, type, record)                                                                         \
  add_search_type("binary", func, type, BranchingBinarySearch<record>, record);                           \
  add_search_type("linear", func, type, LinearSearch<record>, record);                                    \
  add_search_type("avx", func, type, LinearAVX<type COMMA record>, record);                               \
  add_search_type("interpolation", func, type, InterpolationSearch<record>, record);                      \
  add_search_type("exponential", func, type, ExponentialSearch<record>, record);

// template <class SearchClass, int track_errors, bool is_default>
// void execute_32_bit(sosd::Benchmark<uint32_t>& benchmark, bool pareto, bool only_mode,
//                     std::string only, std::string filename) {
//   // Build and probe individual indexes.
//   if constexpr (track_errors != 2){
//     check_only("RMI", benchmark_32_rmi<SearchClass>(benchmark, pareto, filename), benchmark_32_rmi<LinearSearch<track_errors>>(benchmark, pareto, filename));
//     check_only("TS", benchmark_32_ts<SearchClass>(benchmark, pareto), benchmark_32_ts<BranchingBinarySearch<track_errors>>(benchmark, pareto));
//     check_only("PGM", benchmark_32_pgm<SearchClass>(benchmark, pareto), benchmark_32_pgm<LinearSearch<track_errors>>(benchmark, pareto));
//     check_only("DynamicPGM", benchmark_32_dynamic_pgm<SearchClass>(benchmark, pareto), benchmark_32_dynamic_pgm<BranchingBinarySearch<track_errors>>(benchmark, pareto));
//     check_only("ART", benchmark_32_art(benchmark, pareto), benchmark_32_art(benchmark, pareto));
//     check_only("BTree", benchmark_32_btree<SearchClass>(benchmark, pareto), benchmark_32_btree<BranchingBinarySearch<track_errors>>(benchmark, pareto));
//     check_only("FAST", benchmark_32_fast(benchmark, pareto), benchmark_32_fast(benchmark, pareto));
//     check_only("ALEX", benchmark_32_alex<SearchClass>(benchmark, pareto), benchmark_32_alex<ExponentialSearch<track_errors>>(benchmark, pareto));
//   #ifndef __APPLE__
//   #ifndef DISABLE_FST
//     check_only("FST", benchmark_32_fst(benchmark, pareto), benchmark_32_fst(benchmark, pareto));
//   #endif
//   #endif
//     check_only("LIPP", benchmark_32_lipp(benchmark, pareto), benchmark_32_lipp(benchmark, pareto));
//     check_only("MABTree", benchmark_32_mabtree<SearchClass>(benchmark, pareto), benchmark_32_mabtree<ExponentialSearch<track_errors>>(benchmark, pareto));
//   }
//   check_only("FINEdex", benchmark_32_finedex<SearchClass>(benchmark, pareto, filename), benchmark_32_finedex<BranchingBinarySearch<track_errors>>(benchmark, pareto, filename));
//   check_only("XIndex", benchmark_32_xindex<SearchClass>(benchmark, pareto), benchmark_32_xindex<ExponentialSearch<track_errors>>(benchmark, pareto));
//   #ifndef __APPLE__
//     check_only("Wormhole",benchmark_32_wormhole(benchmark, pareto), benchmark_32_wormhole(benchmark, pareto));
//   #endif
// }

template <class SearchClass, int record>
void execute_64_bit(sosd::Benchmark<uint64_t>& benchmark, bool pareto, const std::vector<int>& params, bool only_mode,
                    const std::string& only, const std::string& filename) {
  // Build and probe individual indexes.
  if constexpr (record != 2){
    check_only("RMI", benchmark_64_rmi<SearchClass>(benchmark, pareto, params, filename));
    check_only("TS", benchmark_64_ts<SearchClass>(benchmark, pareto, params));
    check_only("PGM", benchmark_64_pgm<SearchClass>(benchmark, pareto, params));
    check_only("DynamicPGM", benchmark_64_dynamic_pgm<SearchClass>(benchmark, pareto, params));
    check_only("ART", benchmark_64_art(benchmark));
    check_only("BTree", benchmark_64_btree<SearchClass>(benchmark, pareto, params));
    check_only("FAST", benchmark_64_fast(benchmark));
    check_only("ALEX", benchmark_64_alex<SearchClass>(benchmark, pareto, params));
  #ifndef __APPLE__
  #ifndef DISABLE_FST
    check_only("FST", benchmark_64_fst(benchmark, pareto, params));
  #endif
  #endif
    check_only("LIPP", benchmark_64_lipp(benchmark));
    check_only("MABTree", benchmark_64_mabtree<SearchClass>(benchmark, pareto, params));
  }
  check_only("ARTOLC", benchmark_64_artolc(benchmark));
  check_only("FINEdex", benchmark_64_finedex<SearchClass>(benchmark, pareto, params, filename));
  check_only("XIndex", benchmark_64_xindex<SearchClass>(benchmark, pareto, params, filename));
  #ifndef __APPLE__
    check_only("Wormhole",benchmark_64_wormhole(benchmark));
  #endif
}

template <int record>
void execute_64_bit(sosd::Benchmark<uint64_t>& benchmark, bool only_mode,
                    const std::string& only, const std::string& filename) {
  // Build and probe individual indexes.
  if constexpr (record != 2){
    check_only("RMI", benchmark_64_rmi<record>(benchmark, filename));
    check_only("TS", benchmark_64_ts<record>(benchmark, filename));
    check_only("PGM", benchmark_64_pgm<record>(benchmark, filename));
    check_only("DynamicPGM", benchmark_64_dynamic_pgm<record>(benchmark, filename));
    check_only("ART", benchmark_64_art(benchmark));
    check_only("BTree", benchmark_64_btree<record>(benchmark, filename));
    check_only("FAST", benchmark_64_fast(benchmark));
    check_only("ALEX", benchmark_64_alex<record>(benchmark, filename));
  #ifndef __APPLE__
  #ifndef DISABLE_FST
    check_only("FST", benchmark_64_fst(benchmark, filename));
  #endif
  #endif
    check_only("LIPP", benchmark_64_lipp(benchmark));
    check_only("MABTree", benchmark_64_mabtree<record>(benchmark, filename));
  }
  check_only("ARTOLC", benchmark_64_artolc(benchmark));
  check_only("FINEdex", benchmark_64_finedex<record>(benchmark, filename));
  check_only("XIndex", benchmark_64_xindex<record>(benchmark, filename));
  #ifndef __APPLE__
    check_only("Wormhole",benchmark_64_wormhole(benchmark));
  #endif
}

template <class SearchClass, int record>
void execute_string(sosd::Benchmark<std::string>& benchmark, bool pareto, const std::vector<int>& params, bool only_mode,
                    const std::string& only, const std::string& filename) {
  // Build and probe individual indexes.
  if constexpr (record != 2){
    check_only("ART", benchmark_string_art(benchmark));
  #ifndef __APPLE__
  #ifndef DISABLE_FST
    check_only("FST", benchmark_string_fst(benchmark, pareto, params));
  #endif
  #endif
  }
  check_only("ARTOLC", benchmark_string_artolc(benchmark));
  check_only("SIndex", benchmark_string_sindex<SearchClass>(benchmark, pareto, params));
  #ifndef __APPLE__
    check_only("Wormhole",benchmark_string_wormhole(benchmark));
  #endif
}

template <int record>
void execute_string(sosd::Benchmark<std::string>& benchmark, bool only_mode,
                    const std::string& only, const std::string& filename) {
  // Build and probe individual indexes.
  if constexpr (record != 2){
    check_only("ART", benchmark_string_art(benchmark));
  #ifndef __APPLE__
  #ifndef DISABLE_FST
    check_only("FST", benchmark_string_fst(benchmark, filename));
  #endif
  #endif
  }
  check_only("ARTOLC", benchmark_string_artolc(benchmark));
  check_only("SIndex", benchmark_string_sindex<record>(benchmark, filename));
  #ifndef __APPLE__
    check_only("Wormhole",benchmark_string_wormhole(benchmark));
  #endif
}

int main(int argc, char* argv[]) {
  cxxopts::Options options("benchmark", "Searching on sorted data benchmark");
  options.positional_help("<data> <ops>");
  options.add_options()("data", "Data file with keys",
                        cxxopts::value<std::string>())(
      "ops", "Workload file with operations", cxxopts::value<std::string>())(
      "help", "Displays help")(
      "t,threads", "Number of lookup threads",
      cxxopts::value<int>()->default_value("1"))(
      "through", "Measure throughput")(
      "r,repeats", "Number of repeats",
      cxxopts::value<int>()->default_value("1"))(
      "b,build", "Only measure and report build times")(
      "only", "Only run the specified index",
      cxxopts::value<std::string>()->default_value(""))(
      "cold-cache", "Clear the CPU cache between each lookup")(
      "pareto", "Run with multiple different sizes for each competitor")(
      "fence", "Execute a memory barrier between each lookup")(
      "errors", "Tracks index errors, and report those instead of lookup times")(
      "verify", "Verify the correctness of execution")(
      "csv", "Output a CSV of results in addition to a text file")(
      "search", "Specify a search type, one of: linear, avx, binary, interpolation, exponential",
      cxxopts::value<std::string>()->default_value("binary"))(
      "params", "Set the parameters of index",
      cxxopts::value<std::vector<int>>()->default_value(""));

  options.parse_positional({"data", "ops"});

  const auto result = options.parse(argc, argv);

  if (result.count("help")) {
    std::cout << options.help({}) << "\n";
    exit(0);
  }

  const bool through = result.count("through");

  const size_t num_repeats = through ? result["repeats"].as<int>() : 1;
  cout << "Repeating lookup code " << num_repeats << " time(s)." << endl;

  const size_t num_threads = result["threads"].as<int>();
  cout << "Using " << num_threads << " thread(s)." << endl;

  const bool build = result.count("build");
  const bool fence = result.count("fence");
  const bool track_errors = result.count("errors");
  const bool verify = result.count("verify");
  const bool cold_cache = result.count("cold-cache");
  const bool csv = result.count("csv");
  const bool pareto = result.count("pareto");
  const std::string filename = result["data"].as<std::string>();
  const std::string ops = result["ops"].as<std::string>();
  const std::string search_type = result["search"].as<std::string>();
  const bool only_mode = result.count("only") || std::getenv("SOSD_ONLY");
  const std::vector<int> params = result["params"].as<std::vector<int>>();
  std::string only;

  if (result.count("only")) {
    only = result["only"].as<std::string>();
  } else if (std::getenv("SOSD_ONLY")) {
    only = std::string(std::getenv("SOSD_ONLY"));
  } else {
    only = "";
  }

  const DataType type = util::resolve_type(filename);

  if (only_mode)
    cout << "Only executing indexes matching " << only << std::endl;

  // util::set_cpu_affinity(0);

  switch (type) {
    // case DataType::UINT32: {
    //   // Create benchmark.
    //   if (track_errors){
    //     if (num_threads > 1){
    //       add_search_types(execute_32_bit, uint32_t, 2);
    //     } else {
    //       add_search_types(execute_32_bit, uint32_t, 1);
    //     }
    //   }
    //   else{
    //     add_search_types(execute_32_bit, uint32_t, 0);
    //   }
    //   break;
    // }

    case DataType::UINT64: {
      // Create benchmark.
      if (track_errors){
        if (num_threads > 1){
          add_search_types(execute_64_bit, uint64_t, 2);
        } else {
          add_search_types(execute_64_bit, uint64_t, 1);
        }
      }
      else{
        add_search_types(execute_64_bit, uint64_t, 0);
      }
      break;
    }

    case DataType::STRING: {
      // Create benchmark.
      if (track_errors){
        if (num_threads > 1){
          add_search_types(execute_string, std::string, 2);
        } else{
          add_search_types(execute_string, std::string, 1);
        }
      }
      else{
        add_search_types(execute_string, std::string, 0);
      }
      break;
    }
  }

  return 0;
}
