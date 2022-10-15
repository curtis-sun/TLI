#include <iostream>
#include <random>

#include "util.h"
#include "utils/cxxopts.hpp"
#include "competitors/finedex/include/stx/btree_multimap.h"

using namespace std;

// We ensure that there are no more qualifying entries for a given lookup
// than specified here.
// Setting a low value (e.g., 100) here ensures that the checksum computation
// doesn't dominate performance.
constexpr size_t max_num_qualifying = 100;

// Maximum number of retries to find a lookup key that has at most
// `max_num_qualifying entries`.
constexpr size_t max_num_retries = 100;

static mt19937_64 g(42);

enum InsertPat { Equality = 0, Delta = 1, Hotspot = 2 };

vector<size_t> generate_permute(size_t lo, size_t hi, bool is_shuffle=false){
  vector<size_t> permute;
  permute.reserve(hi - lo);
  for (size_t i = lo; i < hi; i ++){
    permute.push_back(i);
  }
  if (is_shuffle){
    shuffle(permute.begin(), permute.end(), g);
  }
  return permute;
}

// Generate queries compatible with `negative_lookup_ratio` 
// and `range_query_ratio`, and manipulate the scan number 
// of range queries with `max_num`.
template <class KeyType>
void generate_equality_lookups(const string& filename, vector<Operation<KeyType>>& ops, util::FastRandom& ranny,
    std::multimap<KeyType, uint64_t>& data_map, vector<KeyValue<KeyType>>& data_vec,
    const double negative_lookup_ratio, bool is_insert, const size_t max_num = 100, const double error = 0.05) {
  bool flag = filename.find("fb_200M_uint64") != std::string::npos;
  for (size_t i = 0; i < ops.size(); i ++){
    if (ops[i].op == util::INSERT){
      data_map.emplace(static_cast<KeyType>(ops[i].lo_key), static_cast<uint64_t>(ops[i].result));
      KeyValue<KeyType> kv;
      kv.key = ops[i].lo_key;
      kv.value = ops[i].result;
      data_vec.push_back(kv);
      continue;
    }
    if (ops[i].op == util::LOOKUP){
      // Required to generate negative lookups within data domain.
      KeyType min_key, max_key;
      if (is_insert){
        min_key = data_map.begin()->first;
        max_key = data_map.rbegin()->first;
        if constexpr (std::is_same<KeyType, uint64_t>::value){
          if (flag && max_key > 77308821508){
            max_key = 77308821508;
          }
        }
      } 
      else{
        min_key = data_vec[0].key;
        max_key = data_vec[data_vec.size() - 1].key;
      }

      if constexpr (!std::is_same<KeyType, std::string>::value){
        if (negative_lookup_ratio > 0 &&
          ranny.ScaleFactor() < negative_lookup_ratio) {
          // Generate negative lookup.
          KeyType negative_lookup;
          bool is_exist = true;
          while (is_exist) {
            // Draw lookup key from within data domain.
            negative_lookup = (ranny.ScaleFactor() * (max_key - min_key)) + min_key;
            if (is_insert){
              is_exist = data_map.find(negative_lookup) != data_map.end();
            }
            else{
              auto it = std::lower_bound(data_vec.begin(), data_vec.end(), negative_lookup, [](const KeyValue<KeyType>& lhs, const KeyType& lookup_key) {
                           return lhs.key < lookup_key;
                         });
              is_exist = it != data_vec.end() && it->key == negative_lookup;
            }
          }
          ops[i].lo_key = negative_lookup;
          ops[i].result = util::NOT_FOUND;
          continue;
        }
      }

      // Generate positive lookup.

      // Draw lookup key from existing keys.
      const uint64_t offset = ranny.RandUint32(0, data_vec.size() - 1);
      const KeyType lookup_key = data_vec[offset].key;
      ops[i].lo_key = lookup_key;
      ops[i].result = 0;
      continue;
    }
    if (ops[i].op == util::RANGE_QUERY){
      size_t num_retries = 0;
      bool generated = false;

      KeyType lo_key, hi_key;
      size_t num_qualifying, min_num = (1 - error) * max_num;
      uint64_t result;

      while(!generated){
        KeyType min_key, max_key; 
        if (is_insert){
          min_key = data_map.begin()->first;
          auto tmp_it = data_map.end();
          advance(tmp_it, - max_num - 1);
          max_key = tmp_it->first;
        }
        else{
          min_key = data_vec[0].key;
          max_key = data_vec[data_vec.size() - max_num - 1].key;
        }

        // Draw lookup key from within data domain.
        if constexpr (std::is_same<KeyType, std::string>::value) {
          while(true){
            const uint64_t offset = ranny.RandUint32(0, data_vec.size() - 1);
            lo_key = data_vec[offset].key;
            if (lo_key > max_key){
              ++num_retries;
              if (num_retries > max_num_retries)
                util::fail("generate_equality_lookups: exceeded max number of retries");
              continue;
            }
            break;
          }
        }
        else{
          lo_key = (ranny.ScaleFactor() * (max_key - min_key)) + min_key;
        }

        num_qualifying = 0;
        result = 0;
        if (is_insert){
          auto lo = data_map.lower_bound(lo_key);
        
          auto tmp_it = lo;
          advance(tmp_it, (1 - ranny.ScaleFactor() * error) * max_num);
          hi_key = tmp_it->first;
          
          while (lo != data_map.end() && lo->first <= hi_key) {
            result += lo->second;
            ++num_qualifying;
            ++lo;
          }
        }
        else{
          auto it = std::lower_bound(data_vec.begin(), data_vec.end(), lo_key, [](const KeyValue<KeyType>& lhs, const KeyType& lookup_key) {
                            return lhs.key < lookup_key;
                          });
          hi_key = (it + (1 - ranny.ScaleFactor() * error) * max_num)->key;
          while(it != data_vec.end() && it->key <= hi_key){
            result += it->value;
            ++num_qualifying;
            ++it;
          }
        }

        if (num_qualifying > max_num || num_qualifying < min_num) {
          // Too many or too few qualifying entries.
          ++num_retries;
          if (num_retries > max_num_retries)
            util::fail("generate_equality_lookups: exceeded max number of retries");
          // Try a different lookup key.
          continue;
        }
        ops[i].lo_key = lo_key;
        ops[i].hi_key = hi_key;
        ops[i].result = result;
        generated = true;
      }
      continue;
    }
    util::fail("unknown operation");
  }
}

// Generate insertions and bulk loads 
// compatible with `insert_ratio` and 
// `insert_pattern`.
template <class KeyType>
void generate_inserts(vector<Operation<KeyType>>& ops, vector<KeyValue<KeyType>>& bulk_loads, const vector<size_t>& op_id,
    const vector<KeyValue<KeyType>>& data_vec,
    const size_t num_insert, const InsertPat pat, const double hotspot_ratio) {
  size_t lo = 0, hi = data_vec.size();
  switch (pat) {
    case InsertPat::Equality: {
      break;
    }

    case InsertPat::Delta: {
      lo = hi - num_insert;
      break;
    }

    case InsertPat::Hotspot: {
      lo = 0.3 * data_vec.size();
      hi = lo + hotspot_ratio * data_vec.size();
      break;
    }

    default: {
      util::fail("undefined insert pattern found.");
    }
  }

  bulk_loads.insert(bulk_loads.end(), data_vec.begin(), data_vec.begin() + lo);

  if (pat == InsertPat::Delta) {
    for (size_t i = 0; i < num_insert; i ++){
      ops[op_id[i]].op = util::INSERT;
      ops[op_id[i]].lo_key = data_vec[lo + i].key;
      ops[op_id[i]].result = data_vec[lo + i].value;
    }
  }
  else {
    vector<size_t> kv_id = generate_permute(lo, hi, true);
    size_t num = hi - lo;
    for (size_t i = 0; i < num; i ++){
      auto kv = data_vec[kv_id[i]];
      if (i < num_insert){
        ops[op_id[i]].op = util::INSERT;
        ops[op_id[i]].lo_key = kv.key;
        ops[op_id[i]].result = kv.value;
      }
      else{
        bulk_loads.push_back(kv);
      }
    }
    sort(bulk_loads.begin() + lo, bulk_loads.end(), [](const KeyValue<KeyType>& kv1, const KeyValue<KeyType>& kv2)->bool {
                                                        return kv1.key < kv2.key;});
  }

  bulk_loads.insert(bulk_loads.end(), data_vec.begin() + hi, data_vec.end());
}

const string to_nice_number(uint64_t num) {
  const uint64_t THOUSAND = 1000;
  const uint64_t MILLION = 1000 * THOUSAND;
  const uint64_t BILLION = 1000 * MILLION;

  if (num >= BILLION && (num / BILLION) * BILLION == num) {
    return to_string(num / BILLION) + "B";
  }
  if (num >= MILLION && (num / MILLION) * MILLION == num) {
    return to_string(num / MILLION) + "M";
  }
  if (num >= THOUSAND && (num / THOUSAND) * THOUSAND == num) {
    return to_string(num / THOUSAND) + "K";
  }
  return to_string(num);
}

template <class KeyType>
void print_op_stats(
  vector<Operation<KeyType>> const* ops, size_t thread_num) {
  for (size_t i = 0; i < thread_num; i ++){
    size_t negative_count = 0, lookup_count = 0, rq_count = 0, insert_count = 0;
    for (const auto& op: ops[i]){
      if (op.op == util::LOOKUP){
        if (op.result == util::NOT_FOUND){
          ++negative_count;
        }
        ++lookup_count;
        continue;
      }
      if (op.op == util::RANGE_QUERY){
        ++rq_count;
        continue;
      }
      if (op.op == util::INSERT){
        ++insert_count;
        continue;
      }
    }
    cout << "thread's operation count: " << ops[i].size() << endl;
    cout << "negative lookup ratio: " << static_cast<double>(negative_count) / lookup_count << endl;
    cout << "range query ratio: " << static_cast<double>(rq_count) / ops[i].size() << endl;
    cout << "insert ratio: " << static_cast<double>(insert_count) / ops[i].size() << endl;
  }
}

template <class KeyType>
void generate(const string& filename, size_t op_cnt, 
              double range_query_ratio, double negative_lookup_ratio, double insert_ratio,
              InsertPat pat, double hotspot_ratio, 
              size_t thread_num, bool mix, size_t bulkload_cnt){
  util::FastRandom ranny(42);
  // Load data.
  const vector<KeyType> keys = util::load_data<KeyType>(filename);

  if (!is_sorted(keys.begin(), keys.end()))
    util::fail("keys have to be sorted");

  // Generate name for benchmark.
  string op_filename = filename + "_ops_" + to_nice_number(op_cnt) + "_" 
                    + to_string(range_query_ratio) + "rq_"
                    + to_string(negative_lookup_ratio) + "nl_"
                    + to_string(insert_ratio) + "i";
  if (insert_ratio > 0)
    op_filename += "_" + to_string(pat) + "m";
  if (pat == InsertPat::Hotspot)
    op_filename += "_" + to_string(hotspot_ratio) + "h";
  if (thread_num > 1){
    op_filename += "_" + to_string(thread_num) + "t";
  }
  if (mix){
    op_filename += "_mix";
  }
  if (bulkload_cnt != size_t(-1)){
    op_filename += "_" + to_nice_number(bulkload_cnt) + "bulkload";
  }
  string bulkload_filename = op_filename + "_bulkload"; 

  // Add artificial values to original keys.
  vector<KeyValue<KeyType>> org_vec = util::add_values(keys);

  size_t insert_cnt = op_cnt * insert_ratio;
  size_t range_query_cnt = op_cnt * range_query_ratio;
  size_t lookup_cnt = op_cnt - insert_cnt - range_query_cnt;
  
  vector<KeyValue<KeyType>> bulk_loads;
  bulk_loads.reserve(org_vec.size() - insert_cnt);
  vector<Operation<KeyType>> tot_ops(op_cnt);
  
  vector<size_t> op_id = generate_permute(0, op_cnt, mix);
  if (insert_ratio > 0) {
    if (mix){
      sort(op_id.begin(), op_id.begin() + insert_cnt);
    }
    generate_inserts(tot_ops, bulk_loads, op_id,
      org_vec, insert_cnt, pat, hotspot_ratio);

    if (bulkload_cnt != size_t(-1)){
      vector<KeyValue<KeyType>> sample_bulk_loads;
      sample(bulk_loads.begin(), bulk_loads.end(), back_inserter(sample_bulk_loads), bulkload_cnt, g);
      bulk_loads.swap(sample_bulk_loads);
    }
  }
  else if (bulkload_cnt != size_t(-1)){
    sample(org_vec.begin(), org_vec.end(), back_inserter(bulk_loads), bulkload_cnt, g);
  }
  else {
    bulk_loads.swap(org_vec);
  }

  for (size_t j = insert_cnt; j < insert_cnt + lookup_cnt; j ++){
    tot_ops[op_id[j]].op = util::LOOKUP;
  }
  for (size_t j = insert_cnt + lookup_cnt; j < op_cnt; j ++){
    tot_ops[op_id[j]].op = util::RANGE_QUERY;
  }

  vector<pair<KeyType, uint64_t>> insert_vecs[thread_num];
  vector<Operation<KeyType>> ops[thread_num];
  if (thread_num > 1){
    for (const auto& e: tot_ops){
      size_t i = ranny.RandUint32(0, thread_num - 1);
      ops[i].push_back(e);
      if (e.op == util::INSERT){
        insert_vecs[i].emplace_back(e.lo_key, e.result);
      }
    }
    // size_t nxt_id = cur_id + 1;
    // while(nxt_id < org_vec.size() && org_vec[nxt_id].key == org_vec[nxt_id - 1].key){
    //   ++ nxt_id;
    // }
  }
  else{
    ops[0] = tot_ops;
  }


  for (size_t i = 0; i < thread_num; i ++){
    // Generate benchmarks.
    
    vector<KeyValue<KeyType>> data_vec = bulk_loads;
    std::multimap<KeyType, uint64_t> data_map;
    if (insert_ratio > 0){
      vector<std::pair<KeyType, uint64_t>> other_vec;
      for (const auto& kv: bulk_loads){
        other_vec.emplace_back(kv.key, kv.value);
      }
      for (size_t j = 0; j < thread_num; ++ j){
        if (j == i){
          continue;
        }
        other_vec.insert(other_vec.end(), insert_vecs[j].begin(), insert_vecs[j].end());
      }
      sort(other_vec.begin(), other_vec.end());
      data_map.insert(other_vec.begin(), other_vec.end());
    }
    
    generate_equality_lookups(filename, ops[i], ranny,
      data_map, data_vec, negative_lookup_ratio, insert_ratio > 0);
  }
  print_op_stats(ops, thread_num);

  if (insert_ratio > 0 || bulkload_cnt != size_t(-1)){
    util::write_data(bulk_loads, bulkload_filename);
  }
  if (thread_num > 1){
    util::write_data_multithread(ops, thread_num, op_filename);
  }
  else{
    util::write_data(ops[0], op_filename);
  }
}

int main(int argc, char* argv[]) {
  cxxopts::Options options("generate", "Generate operations on sorted data");
  options.positional_help("<data> <operation-count>");
  options.add_options()("data", "Data file with keys",
                        cxxopts::value<string>())("bulkload-count", "Bulk loaded data size", 
                               cxxopts::value<size_t>()->default_value(to_string(size_t(-1))))(
      "operation-count", "Number of operations", cxxopts::value<size_t>())(
      "help", "Displays help")("n,negative-lookup-ratio", "Negative lookup ratio", 
                               cxxopts::value<double>()->default_value("0"))(
      "t,thread", "Number of threads", 
                               cxxopts::value<size_t>()->default_value("1"))(
      "s,scan-ratio", "Range query ratio", 
                               cxxopts::value<double>()->default_value("0"))(
      "i,insert-ratio", "Insert ratio", 
                               cxxopts::value<double>()->default_value("0"))(
      "m,insert-pattern", "Specify an insert pattern, one of: equality, delta, hotspot",
                               cxxopts::value<string>()->default_value("equality"))(
      "h,hotspot-ratio", "Hotspot ratio",
                               cxxopts::value<double>()->default_value("0.1"))(
      "mix", "Mix lookups, range queries and inserts together");

  options.parse_positional({"data", "operation-count"});

  const auto result = options.parse(argc, argv);

  if (result.count("help")) {
    cout << options.help({}) << "\n";
    exit(0);
  }

  const bool mix = result.count("mix");

  const string filename = result["data"].as<string>();
  const DataType type = util::resolve_type(filename);
  size_t op_cnt = result["operation-count"].as<size_t>();
  size_t thread_num = result["thread"].as<size_t>();
  size_t bulkload_cnt = result["bulkload-count"].as<size_t>();
  double range_query_ratio = result["scan-ratio"].as<double>(), 
         negative_lookup_ratio = result["negative-lookup-ratio"].as<double>(),
         insert_ratio = result["insert-ratio"].as<double>(),
         hotspot_ratio = result["hotspot-ratio"].as<double>();

  InsertPat pat = InsertPat::Equality; 
  if (insert_ratio > 0){
    const string pat_str = result["insert-pattern"].as<string>();
    if (pat_str == "equality") {}
    else if (pat_str == "delta"){
      pat = InsertPat::Delta;
    }
    else if (pat_str == "hotspot"){
      pat = InsertPat::Hotspot;
    }
    else {
      util::fail("undefined insert pattern found.");
    }
  }

  if (negative_lookup_ratio < 0 || negative_lookup_ratio > 1
  || range_query_ratio < 0 || range_query_ratio > 1
  || insert_ratio < 0 || insert_ratio > 1
  || hotspot_ratio <= 0 || hotspot_ratio > 1) {
    util::fail("workload ratio must be between 0 and 1.");
  }

  switch (type) {
    case DataType::UINT32: {
      generate<uint32_t>(filename, op_cnt, 
                  range_query_ratio, negative_lookup_ratio, insert_ratio, 
                  pat, hotspot_ratio,
                  thread_num, mix, bulkload_cnt);
      break;
    }
    case DataType::UINT64: {
      generate<uint64_t>(filename, op_cnt, 
                  range_query_ratio, negative_lookup_ratio, insert_ratio, 
                  pat, hotspot_ratio,
                  thread_num, mix, bulkload_cnt);
      break;
    }
    case DataType::STRING: {
      generate<std::string>(filename, op_cnt, 
                  range_query_ratio, 0, insert_ratio, 
                  pat, hotspot_ratio,
                  thread_num, mix, bulkload_cnt);
      break;
    }
  }

  return 0;
}
