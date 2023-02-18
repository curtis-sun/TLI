#ifndef TLI_PGM_H
#define TLI_PGM_H

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <vector>

#include "../util.h"
#include "base.h"
#include "pgm_index.hpp"

template <class KeyType, class SearchClass, size_t pgm_error>
class PGM : public Competitor<KeyType, SearchClass> {
 public:
  PGM(const std::vector<int>& params){}
  uint64_t Build(const std::vector<KeyValue<KeyType>>& data, const size_t num_threads) {
    std::vector<KeyType> keys;
    keys.reserve(data.size());
    std::transform(data.begin(), data.end(), std::back_inserter(keys),
                   [](const KeyValue<KeyType>& kv) { return kv.key; });

    uint64_t build_time = util::timing([&] { 
          data_ = data;
          pgm_ = decltype(pgm_)(keys.begin(), keys.end()); 
        });

    return build_time;
  }

  size_t EqualityLookup(const KeyType lookup_key, uint32_t thread_id) const {
    auto approx_range = pgm_.find_approximate_position(lookup_key);
    auto pos = approx_range.pos;
    auto lo = approx_range.lo;
    auto hi = approx_range.hi;
    typedef typename std::vector<KeyValue<KeyType>>::const_iterator Iterator;
    auto it = SearchClass::lower_bound(data_.begin() + lo, data_.begin() + hi, lookup_key,  
                      data_.begin() + pos, std::function<KeyType(Iterator)>([](Iterator it)->KeyType {
                                                      return it->key; }));
    if (it == data_.end() || it->key != lookup_key){
      return util::OVERFLOW;
    }
    return it - data_.begin();
  }

  uint64_t RangeQuery(const KeyType lower_key, const KeyType upper_key, uint32_t thread_id) const {
    auto approx_range = pgm_.find_approximate_position(lower_key);
    auto pos = approx_range.pos;
    auto lo = approx_range.lo;
    auto hi = approx_range.hi;
    typedef typename std::vector<KeyValue<KeyType>>::const_iterator Iterator;
    auto it = SearchClass::lower_bound(data_.begin() + lo, data_.begin() + hi, lower_key,  
                      data_.begin() + pos, std::function<KeyType(Iterator)>([](Iterator it)->KeyType {
                                                      return it->key; }));
    while(it != data_.end() && it->key < lower_key){
      ++it;
    }
    uint64_t result = 0;
    while(it != data_.end() && it->key <= upper_key){
      result += it->value;
      ++it;
    }
    return result;
  }

  std::string name() const { return "PGM"; }

  std::size_t size() const { return pgm_.size_in_bytes() + (sizeof(KeyType) + sizeof(uint64_t)) * data_.size(); }

  bool applicable(bool unique, bool range_query, bool insert, bool multithread, const std::string& ops_filename) const {
    std::string name = SearchClass::name();
    return !insert && name != "LinearAVX" && !multithread;
  }

  std::vector<std::string> variants() const { 
    std::vector<std::string> vec;
    vec.push_back(SearchClass::name());
    vec.push_back(std::to_string(pgm_error));
    return vec;
  }

 private:
  PGMIndex<KeyType, SearchClass, pgm_error, 4> pgm_;
  std::vector<KeyValue<KeyType>> data_;
};

#endif  // TLI_PGM_H
