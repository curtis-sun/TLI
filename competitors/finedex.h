#ifndef SOSDB_FINEDEX_H
#define SOSDB_FINEDEX_H

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <vector>

#include "../util.h"
#include "base.h"
#include "finedex/include/aidel.h"
#include "finedex/include/aidel_impl.h"

namespace sosd_finedex{

template <class KeyType, class SearchClass>
class FINEdex : public Competitor<KeyType, SearchClass> {
 public:
  typedef aidel::AIDEL<KeyType, uint64_t, SearchClass> aidel_type;

  FINEdex(const std::vector<int>& params): max_error(params[0]){
    table = new aidel_type();
  }
  ~FINEdex(){
    delete table;
  }

  uint64_t Build(const std::vector<KeyValue<KeyType>>& data, size_t num_threads) {
    std::vector<KeyType> keys;
    std::vector<uint64_t> values;
    for (const KeyValue<KeyType>& kv : data) {
      keys.push_back(kv.key);
      values.push_back(kv.value);
    }

    num_workers_ = num_threads;

    uint64_t build_time =
        util::timing([&] { 
          table->train(keys, values, max_error);
    });

    return build_time;
  }

  size_t EqualityLookup(const KeyType& lookup_key, uint32_t thread_id) const {
    uint64_t guess;
    result_t res = table->find(lookup_key, guess);
    if (res == result_t::ok){
      return guess;
    }
    return util::OVERFLOW;
  }

  uint64_t RangeQuery(const KeyType& lower_key, const KeyType& upper_key, uint32_t thread_id) const {
    std::vector<std::pair<KeyType, uint64_t>> results;
    table->range_scan(lower_key, upper_key, results);
    uint64_t result = 0;
    for (size_t i = 0; i < results.size(); ++ i){
      result += results[i].second;
    }
    return result;
  }

  void Insert(const KeyValue<KeyType>& data, uint32_t thread_id) {
    table->insert(data.key, data.value);
  }

  std::string name() const { return "FINEdex"; }

  std::size_t size() const { return table->size(); }

  bool applicable(bool unique, bool range_query, bool insert, bool multithread, const std::string& ops_filename) const {
    return !std::is_same<KeyType, std::string>::value && unique && ops_filename.find("1m") == std::string::npos;
  }

  std::vector<std::string> variants() const { 
    std::vector<std::string> vec;
    vec.push_back(SearchClass::name());
    vec.push_back(std::to_string(max_error));
    return vec;
  }

  uint64_t runMultithread(void *(* func)(void *), FGParam *params) {
    pthread_t threads[num_workers_];

    for (size_t worker_i = 0; worker_i < num_workers_; worker_i++) {
      int ret = pthread_create(&threads[worker_i], nullptr, func,
                              (void *)&params[worker_i]);
      if (ret) {
        std::cout << "Error: " << ret << std::endl;
      }
    }

    std::cout << "[micro] prepare data ...\n";
    while (util::ready_threads < num_workers_) sleep(1);
    util::running = true;

    uint64_t timing = util::timing([&] {
      while (util::running)
        ;
    });

    void *status;
    for (size_t i = 0; i < num_workers_; i++) {
      int rc = pthread_join(threads[i], &status);
      if (rc) {
        std::cout << "Error:unable to join, " << rc << std::endl;
      }
    }
    
    return timing;
  }

 private:
  aidel_type* table;
  size_t num_workers_;
  size_t max_error;
};

};

#endif  // SOSDB_FINEDEX_H
