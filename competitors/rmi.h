#pragma once

#include <math.h>

#include "base.h"
#include "rmi/all_rmis.h"

//#define DEBUG_RMI

// RMI with binary search
template <class KeyType, class SearchClass, int rmi_variant, uint64_t build_time, size_t rmi_size,
          const char* namespc, uint64_t (*RMI_FUNC)(uint64_t, size_t*),
          bool (*RMI_LOAD)(char const*), void (*RMI_CLEANUP)()>
class RMI_B: public Competitor<KeyType, SearchClass> {
 public:
 RMI_B(const std::vector<int>& params){}
 uint64_t Build(const std::vector<KeyValue<KeyType>>& data, const size_t num_threads) {
    const std::string rmi_path =
        (std::getenv("SOSD_RMI_PATH") == NULL ? "rmi_data"
                                              : std::getenv("SOSD_RMI_PATH"));
    if (!RMI_LOAD(rmi_path.c_str())) {
      util::fail(
          "Could not load RMI data from rmi_data/ -- either an allocation "
          "failed or the file could not be read.");
    }

    return build_time + util::timing([&] {
      data_ = data;
    });
  }

  size_t EqualityLookup(const KeyType lookup_key, uint32_t thread_id) const {
    size_t error;
    uint64_t guess = RMI_FUNC(lookup_key, &error);

    uint64_t start = (guess < error ? 0 : guess - error);
    uint64_t stop = (guess + error >= data_.size() ? data_.size() : guess + error);

    typedef typename std::vector<KeyValue<KeyType>>::const_iterator Iterator;
    auto it = SearchClass::lower_bound(data_.begin() + start, data_.begin() + stop, lookup_key,  
                      data_.begin() + guess, std::function<KeyType(Iterator)>([](Iterator it)->KeyType {
                                                      return it->key; }));
    if (it == data_.end() || it->key != lookup_key){
      return util::OVERFLOW;
    }
    return it - data_.begin();
  }

  uint64_t RangeQuery(const KeyType lower_key, const KeyType upper_key, uint32_t thread_id) const {
    size_t error;
    uint64_t guess = RMI_FUNC(lower_key, &error);
    uint64_t start = (guess < error ? 0 : guess - error);
    uint64_t stop = (guess + error >= data_.size() ? data_.size() : guess + error);

    typedef typename std::vector<KeyValue<KeyType>>::const_iterator Iterator;
    auto it = SearchClass::lower_bound(data_.begin() + start, data_.begin() + stop, lower_key,  
                      data_.begin() + guess, std::function<KeyType(Iterator)>([](Iterator it)->KeyType {
                                                      return it->key; }));
    uint64_t result = 0;
    while(it != data_.end() && it->key <= upper_key){
      result += it->value;
      ++it;
    }
    return result;
  }

  bool applicable(bool unique, bool range_query, bool insert, bool multithread, const std::string& data_filename) const {
    return !insert && data_filename.find("books_200M_uint32") == std::string::npos && !multithread;
  }

  std::vector<std::string> variants() const { 
    std::vector<std::string> vec;
    vec.push_back(SearchClass::name());
    vec.push_back(std::to_string(rmi_variant));
    return vec;
  }

  std::string name() const { return "RMI"; }

  std::size_t size() const { return rmi_size + (sizeof(KeyType) + sizeof(uint64_t)) * data_.size(); }

  ~RMI_B() { RMI_CLEANUP(); }

 private:
  std::vector<KeyValue<KeyType>> data_;
};
