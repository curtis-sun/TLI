#pragma once

#include <stx/btree_multimap.h>

#include "../utils/tracking_allocator.h"
#include "base.h"

template <class KeyType, class SearchClass, size_t max_node_logsize>
class STXBTree : public Competitor<KeyType, SearchClass> {
 public:
  STXBTree(const std::vector<int>& params)
      : btree_(TrackingAllocator<std::pair<KeyType, uint64_t>>(
            total_allocation_size)) {}
  uint64_t Build(const std::vector<KeyValue<KeyType>>& data, const size_t num_threads) {
    std::vector<std::pair<KeyType, uint64_t>> reformatted_data;
    reformatted_data.reserve(data.size());

    for (const auto& iter : data) {
      reformatted_data.emplace_back(iter.key, iter.value);
    }

    return util::timing([&] {
      btree_.bulk_load(reformatted_data.begin(), reformatted_data.end());
    });
  }

  size_t EqualityLookup(const KeyType lookup_key, uint32_t thread_id) const {
    auto it = btree_.find(lookup_key);

    uint64_t guess;
    if (it == btree_.end()) {
      guess = util::NOT_FOUND;
    } else {
      guess = it->second;
    }

    return guess;
  }

  uint64_t RangeQuery(const KeyType lower_key, const KeyType upper_key, uint32_t thread_id) const {
    auto it = btree_.lower_bound(lower_key);
    uint64_t result = 0;
    while(it != btree_.end() && it->first <= upper_key){
      result += it->second;
      ++it;
    }
    return result;
  }
  
  void Insert(const KeyValue<KeyType>& data, uint32_t thread_id) {
    btree_.insert(data.key, data.value);
  }

  std::vector<std::string> variants() const { 
    std::vector<std::string> vec;
    vec.push_back(SearchClass::name());
    vec.push_back(std::to_string(max_node_logsize));
    return vec;
  }

  bool applicable(bool unique, bool range_query, bool insert, bool multithread, const std::string& ops_filename) {
        return !multithread;
    }

  std::string name() const { return "BTree"; }

  std::size_t size() const {
    return btree_.get_allocator().total_allocation_size;
  }

  template <typename _Key, typename _Data, int max_node_size>
    struct btree_default_map_traits
    {
        static const bool   selfverify = false;
        static const bool   debug = false;
        static const int    leafslots = BTREE_MAX( 8, max_node_size / (sizeof(_Key) + sizeof(_Data)) );
        static const int    innerslots = BTREE_MAX( 8, max_node_size / (sizeof(_Key) + sizeof(void*)) );
    };

 private:
  // Using a multimap here since keys may contain duplicates.
  uint64_t total_allocation_size = 0;
  uint64_t data_size_ = 0;
  stx::btree_multimap<KeyType, uint64_t, SearchClass, std::less<KeyType>,
                      btree_default_map_traits<KeyType, uint64_t, 1 << max_node_logsize>,
                      TrackingAllocator<std::pair<KeyType, uint64_t>>>
      btree_;
};
