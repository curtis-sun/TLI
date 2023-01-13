#pragma once

#include <utility>

#include "./ALEX/src/core/alex.h"
#include "./ALEX/src/core/alex_base.h"
#include "base.h"

template <class KeyType, class SearchClass>
class Alex : public Competitor<KeyType, SearchClass> {
 public:
  Alex(const std::vector<int>& params): max_node_logsize(params[0]){}
  uint64_t Build(const std::vector<KeyValue<KeyType>>& data, const size_t num_threads) {
    std::vector<std::pair<KeyType, uint64_t>> loading_data;
    loading_data.reserve(data.size());
    for (const auto& itm : data) {
      loading_data.push_back(std::make_pair(itm.key, itm.value));
    }
    
    map_.set_max_node_size(1 << max_node_logsize);

    return util::timing(
        [&] { map_.bulk_load(loading_data.data(), loading_data.size()); });
  }

  size_t EqualityLookup(const KeyType lookup_key, uint32_t thread_id) const {
    auto it = map_.find(lookup_key);

    uint64_t guess;
    if (it == map_.cend()) {
      guess = util::NOT_FOUND;
    } else {
      guess = it.payload();
    }

    return guess;
  }

  uint64_t RangeQuery(const KeyType lower_key, const KeyType upper_key, uint32_t thread_id) const {
    auto it = map_.lower_bound(lower_key);
    uint64_t result = 0;
    while(it != map_.cend() && it.key() <= upper_key){
      result += it.payload();
      ++it;
    }
    return result;
  }

  void Insert(const KeyValue<KeyType>& data, uint32_t thread_id) {
    map_.insert(std::make_pair(data.key, data.value));
  }

  std::string name() const { return "ALEX"; }

  std::size_t size() const { return map_.model_size() + map_.data_size(); }

  bool applicable(bool unique, bool range_query, bool insert, bool multithread, const std::string& ops_filename) const {
    // The max_data_node_slots should > 21026, 
    // which is the maximum repeated num in dataset wiki_ts_200M_uint64
    return (ops_filename.find("wiki_ts") == std::string::npos || max_node_logsize >= 20) && !multithread;
  }

  std::vector<std::string> variants() const { 
    std::vector<std::string> vec;
    vec.push_back(SearchClass::name());
    vec.push_back(std::to_string(max_node_logsize));
    return vec;
  }

 private:
  alex::Alex<KeyType, uint64_t, SearchClass> map_;
  size_t max_node_logsize;
};
