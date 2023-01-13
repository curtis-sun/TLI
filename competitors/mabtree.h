#pragma once

#include "./MADEX/src/mabtree.h"

#include "../utils/tracking_allocator.h"
#include "base.h"

template <class KeyType, class SearchClass, size_t max_node_logsize, size_t predict_page_logsize>
class MABTree : public Competitor<KeyType, SearchClass> {
 public:
  MABTree(const std::vector<int>& params)
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
    btree_.insert2(data.key, data.value);
  }

  std::vector<std::string> variants() const { 
    std::vector<std::string> vec;
    vec.push_back(SearchClass::name());
    vec.push_back(std::to_string(max_node_logsize));
    vec.push_back(std::to_string(predict_page_logsize));
    return vec;
  }

  std::string name() const { return "MABTree"; }

  bool applicable(bool unique, bool range_query, bool insert, bool multithread, const std::string& ops_filename) const {
    // The last several keys of AlexNode's key_slots_ are std::numeric_limits<KeyType>::max()
    return SearchClass::name() != "InterpolationSearch" && !multithread;
  }

  std::size_t size() const {
    return btree_.get_allocator().total_allocation_size;
  }

  template <typename _Key, typename _Data, int max_node_size, int predict_page_size>
    struct mabtree_default_traits
    {
        static const bool   selfverify = false;
        static const bool   debug = false;
        static const int    leafslots = BTREE_MAX( 8, max_node_size / (sizeof(_Key) + sizeof(_Data)) );
        static const int    innerslots = BTREE_MAX( 8, max_node_size / (sizeof(_Key) + sizeof(void*)) );
        static const int    page_size = predict_page_size;
    };

 private:
  // Using a multimap here since keys may contain duplicates.
  uint64_t total_allocation_size = 0;
  uint64_t data_size_ = 0;
  mabtree::mabtree<KeyType, uint64_t, SearchClass, std::pair<KeyType, uint64_t>, std::less<KeyType>,
                      mabtree_default_traits<KeyType, uint64_t, 1 << max_node_logsize, 1 << predict_page_logsize>,
                      true, TrackingAllocator<std::pair<KeyType, uint64_t>>, false>
      btree_;
};
