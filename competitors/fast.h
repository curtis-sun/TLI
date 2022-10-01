#pragma once

#include "base.h"
#include "./fast/src/fast.h"

template<class KeyType>
class Fast : public Base<KeyType> {
 public:
  Fast(const std::vector<int>& params){}
  uint64_t Build(const std::vector<KeyValue<KeyType>>& data, size_t num_threads) {
    const auto extract_key = [](KeyValue<KeyType> kv) { return kv.key; };
    std::vector<KeyType> keys;
    keys.reserve(data.size());
    std::transform(data.begin(), data.end(), std::back_inserter(keys),
                   extract_key);

    return util::timing([&] {
      data_ = data;
      fast_.buildFAST(keys.data(), data.size());
    });
  }

  size_t EqualityLookup(const KeyType& lookup_key, uint32_t thread_id) const {
    size_t it = fast_.lower_bound(lookup_key);
    if (it == data_.size() || data_[it].key != lookup_key){
      return util::OVERFLOW;
    }
    return it;
  }

  uint64_t RangeQuery(const KeyType& lower_key, const KeyType& upper_key, uint32_t thread_id) const {
    size_t it = fast_.lower_bound(lower_key);
    uint64_t result = 0;
    while(it != data_.size() && data_[it].key <= upper_key){
      result += data_[it].value;
      ++it;
    }
    return result;
  }

  std::string name() const { return "FAST"; }

  std::size_t size() const { return fast_.size_in_byte() + (sizeof(KeyType) + sizeof(uint64_t)) * data_.size(); }

  bool applicable(bool unique, bool range_query, bool insert, bool multithread, const std::string& _data_filename) const {
    return unique && !insert && !multithread;
  }

 private:
  std::vector<KeyValue<KeyType>> data_;
  FAST<KeyType> fast_;
};