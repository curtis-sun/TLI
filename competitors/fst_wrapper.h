#pragma once

#include <type_traits>

#include "./FST/include/fst.hpp"
#include "base.h"
#include "../util.h"

template <class KeyType>
class FST : public Base<KeyType> {
 public:
  FST(const std::vector<int>& params): sparse_dense_ratio(params[0]){}
  // assume that keys are unique and sorted in ascending order
  uint64_t Build(const std::vector<KeyValue<KeyType>>& data, size_t num_threads) {
    // transform integer keys to strings
    std::vector<std::string> keys;
    keys.reserve(data.size());

    // we'll construct a `values` array, but it seems to be ignored by FST
    // (we always just get the index).
    std::vector<uint64_t> values;
    values.reserve(data.size());
    for (const KeyValue<KeyType>& kv : data) {
      keys.emplace_back(util::convertToString(kv.key));
      values.push_back(kv.value);
    }

    // build fast succinct trie
    return util::timing(
        [&] { 
        keys_.reserve(data.size());
        for (const KeyValue<KeyType>& kv : data) {
          keys_.push_back(kv.key);
        }
        fst_ = std::make_unique<fst::FST>(keys, values, true, sparse_dense_ratio); 
    });
  }

  size_t EqualityLookup(const KeyType& lookup_key, uint32_t thread_id) const {
    // looking up a value greater than the largest value causes a segfault...
    uint64_t guess = 0;
    if (fst_->lookupKey(util::convertToString(lookup_key), guess) && keys_[guess] == lookup_key){
      return guess;
    }
    return util::OVERFLOW;
  }

  uint64_t RangeQuery(const KeyType& lower_key, const KeyType& upper_key, uint32_t thread_id) const {
    auto iterators = fst_->lookupRange(util::convertToString(lower_key), true, 
                                util::convertToString(upper_key), true);
    uint64_t result = 0;
    if (iterators.first.isValid() && keys_[iterators.first.getValue()] < lower_key){
      iterators.first ++;
    }
    while (iterators.first != iterators.second) {
      result += iterators.first.getValue();
      iterators.first ++;
    }
    if (iterators.second.isValid() && keys_[iterators.first.getValue()] <= upper_key){
      result += iterators.second.getValue();
    }
    return result;
  }

  std::string name() const { return "FST"; }

  std::size_t size() const {
    // return used memory in bytes
    return fst_->getMemoryUsage() + sizeof(KeyType) * keys_.size();
  }

  bool applicable(bool unique, bool range_query, bool insert, bool multithread, const std::string& data_filename) {
    // FST only supports unique keys.
    return unique && !insert && !multithread;
  }

  std::vector<std::string> variants() const { 
    std::vector<std::string> vec;
    vec.push_back(std::to_string(sparse_dense_ratio));
    return vec;
  }

 private:
  std::unique_ptr<fst::FST> fst_;
  std::vector<KeyType> keys_;
  size_t sparse_dense_ratio;
};
