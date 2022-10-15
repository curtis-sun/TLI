#pragma once

#include <immintrin.h>
#include <math.h>

#include <algorithm>
#include <dtl/thread.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <regex>

#include "config.h"
#include "searches/branching_binary_search.h"
#include "util.h"
#include "utils/perf_event.h"
#include <boost/chrono.hpp>

#ifdef __linux__
#define checkLinux(x) (x)
#else
#define checkLinux(x) \
  { util::fail("Only supported on Linux."); }
#endif

// Get the CPU affinity for the process.
static const auto cpu_mask = dtl::this_thread::get_cpu_affinity();

namespace sosd {

// KeyType: Controls the type of the key (the value will always be uint64_t)
//          Use uint64_t for 64 bit types and uint32_t for 32 bit types
//          KeyType must implement operator<

// template <bool multithread>
// static uint64_t timing(std::function<void()> fn) {
//   if constexpr (multithread){
//     const auto start = boost::chrono::thread_clock::now();
//     fn();
//     const auto end = boost::chrono::thread_clock::now();
//     return boost::chrono::duration_cast<boost::chrono::nanoseconds>(end - start)
//         .count();
//   }
//   else{
//     return util::timing(fn);
//   }
// }

static volatile bool run_failed;
static uint64_t random_sum;
// Some memory we can read to flush the cache
// NOTE: 2*L3 size of the machine;
static std::vector<uint64_t> memory(26214400 / 8 * 2);

static void wipe_cache(){
  // Make sure that all cache lines from large buffer are loaded
  for (uint64_t& iter : memory) {
    random_sum += iter;
  }
  _mm_mfence();
}

#define timing_end()                                                                                     \
  if constexpr (multithread){                                                                            \
    const auto end_time = boost::chrono::thread_clock::now();                                            \
    timing = boost::chrono::duration_cast<boost::chrono::nanoseconds>(end_time - thread_clock_start)     \
        .count();                                                                                        \
  } else {                                                                                               \
    const auto end_time = std::chrono::high_resolution_clock::now();                                     \
    timing = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - hr_clock_start)             \
        .count();                                                                                        \
  }                  

template <bool multithread, class KeyType, class Index, bool time_each, bool fence, bool clear_cache, bool verify>
static void* DoOpsCoreLoop(void* param) {
  FGParam &thread_param = *(FGParam *)param;
  Index* index = (Index *)thread_param.index;
  const std::vector<Operation<KeyType>> &ops = *(std::vector<Operation<KeyType>> *)thread_param.ops;
  const std::vector<KeyType> &keys = *(std::vector<KeyType> *)thread_param.keys;
  uint64_t *individual_ns = thread_param.individual_ns;
  size_t start = thread_param.start, limit = thread_param.limit;
  uint32_t thread_id = thread_param.thread_id;

  // std::cout << "[micro] Worker" << thread_id << " Ready.\n";
  util::ready_threads++;
  while (!util::running)
    ;

  boost::chrono::thread_clock::time_point thread_clock_start;
  std::chrono::high_resolution_clock::time_point hr_clock_start;
  bool flag = false;
  for (size_t idx = start; idx < limit; ++idx) {
    if (!util::running && !flag){
      flag = true;
      thread_param.op_cnt = idx - start;
    }

    const uint8_t op = ops[idx].op;
    const KeyType lo_key = ops[idx].lo_key;
    const KeyType hi_key = ops[idx].hi_key;
    const uint64_t expected = ops[idx].result;

    if constexpr (clear_cache) {
      wipe_cache();
    }

    uint64_t timing = 0;
    if constexpr (time_each) {
      if constexpr (multithread){
        thread_clock_start = boost::chrono::thread_clock::now();
      }
      else{
        hr_clock_start = std::chrono::high_resolution_clock::now();
      }
    }

    switch (op) {
      case util::LOOKUP: {
        size_t idx = index->EqualityLookup(lo_key, thread_id);
        if constexpr (time_each) {
          timing_end();

          if constexpr (verify) {
            if ((expected == util::NOT_FOUND && idx != util::OVERFLOW) || (expected == 0 && (idx >= keys.size() || keys[idx] != lo_key))){
              std::cerr << "equality lookup returned wrong result:" << std::endl;
              std::cerr << "lookup key: " << lo_key << std::endl;
              std::cerr << "actual index: " << idx << ", expected_negative: " << (expected == util::NOT_FOUND) << std::endl;
              run_failed = true;
            }
          }
        }
        break;
      }

      case util::RANGE_QUERY: {
        uint64_t actual = index->RangeQuery(lo_key, hi_key, thread_id);
        if constexpr (time_each) {
          timing_end();

          if constexpr (verify) {
            if (actual != expected){
              std::cerr << "range query returned wrong result:" << std::endl;
              std::cerr << "low key: " << lo_key << ", high key: " << hi_key << std::endl;
              std::cerr << "actual: " << actual << ", expected: " << expected << std::endl;
              run_failed = true;
            }
          }
        }
        break;
      }

      case util::INSERT: {
        index->Insert({lo_key, expected}, thread_id);
        if constexpr (time_each) {
          timing_end();
        }
        break;
      }

      default: {
        std::cerr << "unknown operation: " << op << std::endl;
        run_failed = true;
      }
    }

    if (run_failed){
      return nullptr;
    }

    if constexpr (time_each){
      individual_ns[idx - start] = timing;
    }
    
    if constexpr (fence) __sync_synchronize();
  }

  if (!flag){
    util::running = false;
    thread_param.op_cnt = limit - start;
  }
  return nullptr;
}

struct LatencyStat {
  double avg, mean_square;
  uint64_t p50, p99, p999, max;
};

template <typename KeyType>
class Benchmark {
 public:
  Benchmark(const std::string& data_filename,
            const std::string& ops_filename,
            const bool through, const bool perf, const bool build, const bool fence,
            const bool cold_cache, const bool track_errors, const bool csv,
            const size_t num_threads, const bool verify)
      : data_filename_(data_filename),
        ops_filename_(ops_filename),
        first_run_(true),
        through_(through),
        perf_(perf),
        build_(build),
        fence_(fence),
        cold_cache_(cold_cache),
        track_errors_(track_errors),
        csv_(csv),
        num_threads_(num_threads),
        verify_(verify) {
    if ((int)cold_cache + (int)perf + (int)fence > 1) {
      util::fail(
          "Can only specify one of cold cache, perf counters, or fence.");
    }

    static constexpr const char* prefix = "data/";
    dataset_name_ = ops_filename.data();
    dataset_name_.erase(
        dataset_name_.begin(),
        dataset_name_.begin() + dataset_name_.find(prefix) + strlen(prefix));

    // Load data.
    keys_ = util::load_data<KeyType>(data_filename_);

    // Load lookups.
    if (num_threads_ > 1){
      ops_ = util::load_data_multithread<Operation<KeyType>>(ops_filename_);
    }
    else{
      ops_.push_back(util::load_data<Operation<KeyType>>(ops_filename_));
    }

    bool is_mix = dataset_name_.find("mix") != std::string::npos;

    std::regex rq_pat("(\\d+\\.\\d*)rq"), i_pat("(\\d+\\.\\d*)i");
    std::smatch rq_result, i_result;
    std::regex_search(dataset_name_, rq_result, rq_pat);
    is_range_query_ = std::stod(rq_result[1]) > 0;

    std::regex_search(dataset_name_, i_result, i_pat);
    insert_ratio_ = std::stod(i_result[1]);
    flag_ = is_mix || (insert_ratio_ == 0) || (insert_ratio_ == 1);

    if (insert_ratio_ > 0 || dataset_name_.find("bulkload") != std::string::npos) {
      std::string bl_filename = ops_filename_ + "_bulkload";
      index_data_ = util::load_data<KeyValue<KeyType>>(bl_filename);
    }
    else {
      if (!is_sorted(keys_.begin(), keys_.end()))
        util::fail("keys have to be sorted");
      // Add artificial values to keys.
      index_data_ = util::add_values(keys_);
    }

    bound_points.resize(3 * num_threads_);
    size_t tot_cnt = 0, insert_cnt = 0;
    for (size_t i = 0; i < num_threads_; ++ i){
      bound_points[3 * i] = 0;
      if (!flag_){
        bound_points[3 * i + 1] = std::lower_bound(ops_[i].begin(), ops_[i].end(), 1, 
                                                  [](const Operation<KeyType>& e, const int key){
                                                      return (e.op != util::INSERT) < key;
                                                    }) - ops_[i].begin();
        insert_cnt += bound_points[3 * i + 1];
      }
      bound_points[3 * i + 2] = ops_[i].size();
      tot_cnt += bound_points[3 * i + 2];
    };
    if (flag_){
      individual_ns = new uint64_t[tot_cnt];
    }
    else{
      individual_ns = new uint64_t[std::max(insert_cnt, tot_cnt - insert_cnt)];
    }

    // Check whether keys are unique.
    unique_keys_ = util::is_unique(keys_);
    if (unique_keys_)
      std::cout << "data is unique" << std::endl;
    else
      std::cout << "data contains duplicates" << std::endl;

    if (cold_cache_){
      util::FastRandom ranny(8128);
      for (uint64_t& iter : memory) {
        iter = ranny.RandUint32();
      }
    }
  }
  ~Benchmark(){
    delete[] individual_ns;
  }

  template <class Index>
  void Run(const std::vector<int>& params = std::vector<int>()) {
    // Build index.
    Index* index = new Index(params);

    if (!index->applicable(unique_keys_, is_range_query_, insert_ratio_ > 0, num_threads_ > 1, data_filename_)) {
      std::cout << "index " << index->name() << " is not applicable"
                << std::endl;
      delete index;
      return;
    }

    std::cout << "rsum was: " << random_sum << std::endl;
    random_sum = 0;
    // clean_cache();
    run_failed = false;

    if (build_ && perf_){
      checkLinux(({
          BenchmarkParameters params;
          PerfEventBlock e(1, params, &perf_metrics, /*printHeader=*/first_run_);
          build_ns_ = index->Build(index_data_, num_threads_);
        }));
    }
    else{
      build_ns_ = index->Build(index_data_, num_threads_);
    }

    // Do equality lookups.
    if constexpr (!sosd_config::fast_mode) {
      if (through_) {
        if (fence_) {
          DoOps<Index, false, true, false, false>(index);
        } else{
          DoOps<Index, false, false, false, false>(index);
        }
      } else if (cold_cache_) {
        if (num_threads_ > 1)
          util::fail("cold cache not supported with multiple threads");
        if (verify_){
          DoOps<Index, true, false, true, true>(index);
        } else{
          DoOps<Index, true, false, true, false>(index);
        }
      } else if (fence_) {
        if (verify_){
          DoOps<Index, true, true, false, true>(index);
        } else {
          DoOps<Index, true, true, false, false>(index);
        }
      } else {
        if (verify_){
          DoOps<Index, true, false, false, true>(index);
        } else {
          DoOps<Index, true, false, false, false>(index);
        }
      }
    } else {
      if (perf_ || cold_cache_ || fence_ || verify_) {
        util::fail(
            "Perf, cold cache, fence, and verify mode require full builds. Disable "
            "fast mode.");
      }
      DoOps<Index, true, false, false, false>(index);
    }
    PrintResult(index);

    first_run_ = false;
    delete index;
  }

 private:
  // void clean_cache(){
  //   [[maybe_unused]] int res = system("echo 3 > /proc/sys/vm/drop_caches");
  // }

  template <class Index, bool time_each, bool fence, bool clear_cache, bool verify>
  void DoOps(Index* index) {
    if (build_) return;

    if constexpr (time_each){
      latencies_.clear();
      if (track_errors_){
        search_bounds_.clear();
        search_times_.clear();
        search_latencies_.clear();
      }
    }
    else{
      throughputs_.clear();
    }

    if (perf_){
      perf_metrics.clear();
    }

    for (size_t i = 0; i < 2 - flag_; ++i){
      if constexpr (time_each){
        if (track_errors_){
          index->initSearch();
        }
      }

      FGParam fg_params[num_threads_];
      // check if parameters are cacheline aligned
      for (size_t worker_i = 0; worker_i < num_threads_; ++ worker_i) {
        if ((uint64_t)(&(fg_params[worker_i])) % CACHELINE_SIZE != 0) {
          std::cerr << "wrong parameter address: " << &(fg_params[worker_i]) << std::endl;
        }
      }

      util::running = false;
      util::ready_threads = 0;
      size_t exe_cnt = 0;
      for (size_t worker_i = 0; worker_i < num_threads_; worker_i++) {
        fg_params[worker_i].index = index;
        fg_params[worker_i].ops = &ops_[worker_i];
        fg_params[worker_i].keys = &keys_;
        fg_params[worker_i].individual_ns = individual_ns + exe_cnt;
        fg_params[worker_i].thread_id = worker_i;
        fg_params[worker_i].start = bound_points[3 * worker_i + i];
        fg_params[worker_i].limit = bound_points[3 * worker_i + i + 1 + flag_];

        exe_cnt += fg_params[worker_i].limit - fg_params[worker_i].start;
      }

      uint64_t timing;
      if (num_threads_ > 1){
        timing = index->runMultithread(DoOpsCoreLoop<true, KeyType, Index, time_each, fence, clear_cache, verify>, fg_params);
      }
      else{
        util::running = true;
        timing = util::timing([&] {
          if (perf_){
            checkLinux(({
              BenchmarkParameters params;
              PerfEventBlock e(exe_cnt, params, &perf_metrics, /*printHeader=*/first_run_);
              DoOpsCoreLoop<false, KeyType, Index, time_each, fence, clear_cache, verify>(fg_params);
            }));
          }
          else{
            DoOpsCoreLoop<false, KeyType, Index, time_each, fence, clear_cache, verify>(fg_params);
          }
        });
      }

      if (run_failed){
        return;
      }

      if constexpr (time_each){
        LatencyStat stat;
        double individual_ns_sum = 0;
        double individual_square_sum = 0;
        for (size_t i = 0; i < exe_cnt; i ++){
          individual_square_sum += individual_ns[i] * individual_ns[i];
          individual_ns_sum += individual_ns[i];
        }
        stat.mean_square = sqrt((individual_square_sum - individual_ns_sum * individual_ns_sum / exe_cnt) / (exe_cnt - 1));
        stat.avg = individual_ns_sum / exe_cnt;
        std::sort(individual_ns, individual_ns + exe_cnt);
        stat.p50 = individual_ns[size_t(exe_cnt * 0.5)];
        stat.p99 = individual_ns[size_t(exe_cnt * 0.99)];
        stat.p999 = individual_ns[size_t(exe_cnt * 0.999)];
        stat.max = individual_ns[exe_cnt - 1];
        latencies_.push_back(stat);
        if (track_errors_){
          search_bounds_.push_back(index->searchBound());
          search_times_.push_back(index->searchAverageTime());
          search_latencies_.push_back(index->searchLatency(exe_cnt));
        }
      }
      else{
        double throughput = 0;
        for (size_t worker_i = 0; worker_i < num_threads_; worker_i++) {
          throughput += fg_params[worker_i].op_cnt;
        }
        throughput = throughput * 1e3 / timing;
        throughputs_.push_back(throughput);
      }
    }
  }

  template <class Index>
  void PrintResult(const Index* index) {
    if (run_failed){
      return;
    }

    std::cout << "RESULT: " << index->name() << ","
                << build_ns_ / 1000 << "," << index->size();
                
    if (!build_) {
      if (through_){
        for (auto t: throughputs_){
          std::cout << "," << t;
        }
      } 
      else{
        for (auto l: latencies_){
          std::cout << "," << l.avg << "," << l.p50 << "," << l.p99 << "," << l.p999 << "," << l.max << "," << l.mean_square;
        }

        if (track_errors_){
          for (unsigned int i = 0; i < search_times_.size(); i ++){
            std::cout << "," << search_times_[i] << "," << search_latencies_[i] << "," << search_bounds_[i];
          }
        }
      }

      if (perf_){
        for (const auto& p: perf_metrics){
          std::cout << "," << p;
        }
      }  
    }

    for (auto str: index->variants()){
      std::cout << "," << str;
    } 
    std::cout << std::endl;

    if (csv_) {
      PrintResultCSV(index);
    }
  }

  template <class Index>
  void PrintResultCSV(const Index* index) {
    const std::string filename =
        "./results/" + dataset_name_ + "_results_table.csv";

    std::ofstream fout(filename, std::ofstream::out | std::ofstream::app);

    if (!fout.is_open()) {
      std::cerr << "Failure to print CSV on " << filename << std::endl;
      return;
    }

    fout << index->name() << ","
                << build_ns_ / 1000 << "," << index->size();

    if (!build_) {
      if (through_){
        for (auto t: throughputs_){
          fout << "," << t;
        }
      } 
      else{
        for (auto l: latencies_){
          fout << "," << l.avg << "," << l.p50 << "," << l.p99 << "," << l.p999 << "," << l.max << "," << l.mean_square;
        }

        if (track_errors_){
          for (unsigned int i = 0; i < search_times_.size(); i ++){
            fout << "," << search_times_[i] << "," << search_latencies_[i] << "," << search_bounds_[i];
          }
        }
      }  

      if (perf_){
        for (const auto& p: perf_metrics){
          fout << "," << p;
        }
      }
    }

    for (auto str: index->variants()){
      fout << "," << str;
    }
    fout << std::endl; 
    fout.close();
    return;
  }

  const std::string data_filename_;
  const std::string ops_filename_;
  std::string dataset_name_;
  std::vector<KeyType> keys_;
  std::vector<KeyValue<KeyType>> index_data_;
  bool unique_keys_;
  bool is_range_query_;
  double insert_ratio_;
  std::vector<std::vector<Operation<KeyType>>> ops_;
  std::vector<size_t> bound_points;
  size_t flag_;
  // Metrics
  uint64_t build_ns_;
  std::vector<LatencyStat> latencies_;
  uint64_t* individual_ns;
  std::vector<double> throughputs_;
  std::vector<double> search_bounds_;
  std::vector<double> search_times_;
  std::vector<double> search_latencies_;
  std::vector<std::string> perf_metrics;
  // Used to only print profiling header information for first run.
  bool first_run_;
  bool through_;
  bool perf_;
  bool build_;
  bool fence_;
  bool measure_each_;
  bool cold_cache_;
  bool track_errors_;
  bool csv_;
  bool verify_;
  // Number of lookup threads.
  const size_t num_threads_;
};

}  // namespace sosd
