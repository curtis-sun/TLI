#pragma once

#include "../util.h"
#include "base.h"
#include "ts/builder.h"
#include "ts/ts.h"

template <class KeyType, class SearchClass>
class TS : public Competitor<KeyType, SearchClass> {
 public:
  TS(const std::vector<int>& params): spline_max_error(params[0]){}
  uint64_t Build(const std::vector<KeyValue<KeyType>>& data, const size_t num_threads) {
    auto min = std::numeric_limits<KeyType>::min();
    auto max = std::numeric_limits<KeyType>::max();
    if (data.size() > 0) {
      min = data.front().key;
      max = data.back().key;
    }

    return util::timing([&] {
      data_ = data;
      
      ts::Builder<KeyType, SearchClass> tsb(min, max, spline_max_error);
      for (const auto& key_and_value : data) tsb.AddKey(key_and_value.key);
      ts_ = tsb.Finalize();
    });
  }

  size_t EqualityLookup(const KeyType lookup_key, uint32_t thread_id) const {
    const ts::SearchBound sb = ts_.GetSearchBound(lookup_key);
    typedef typename std::vector<KeyValue<KeyType>>::const_iterator Iterator;
    auto it = SearchClass::lower_bound(data_.begin() + sb.begin, data_.begin() + sb.end, lookup_key,  
                      data_.begin() + sb.begin, std::function<KeyType(Iterator)>([](Iterator it)->KeyType {
                                                      return it->key; }));
    if (it == data_.end() || it->key != lookup_key){
      return util::OVERFLOW;
    }
    return it - data_.begin();
  }

  uint64_t RangeQuery(const KeyType lower_key, const KeyType upper_key, uint32_t thread_id) const {
    const ts::SearchBound sb = ts_.GetSearchBound(lower_key);
    typedef typename std::vector<KeyValue<KeyType>>::const_iterator Iterator;
    auto it = SearchClass::lower_bound(data_.begin() + sb.begin, data_.begin() + sb.end, lower_key,  
                      data_.begin() + sb.begin, std::function<KeyType(Iterator)>([](Iterator it)->KeyType {
                                                      return it->key; }));
    uint64_t result = 0;
    while(it != data_.end() && it->key <= upper_key){
      result += it->value;
      ++it;
    }
    return result;
  }

  bool applicable(bool unique, bool range_query, bool insert, bool multithread, const std::string& ops_filename) const {
    std::string name = SearchClass::name();
    return !insert && name != "LinearAVX" && !multithread;
  }

  std::vector<std::string> variants() const { 
    std::vector<std::string> vec;
    vec.push_back(SearchClass::name());
    vec.push_back(std::to_string(spline_max_error));
    return vec;
  }

  std::string name() const { return "TS"; }

  std::size_t size() const { return ts_.GetSize() + (sizeof(KeyType) + sizeof(uint64_t)) * data_.size(); }

 private:

  ts::TrieSpline<KeyType, SearchClass> ts_;
  std::vector<KeyValue<KeyType>> data_;
  size_t spline_max_error;
};
