#ifndef SOSDB_XINDEX_H
#define SOSDB_XINDEX_H

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <vector>

#include "../util.h"
#include "base.h"
#include "xindex/xindex.h"
#include "xindex/xindex_impl.h"

namespace sosd_xindex{

template <class KeyType>
class Key {
  typedef std::array<double, 1> model_key_t;

 public:
  static constexpr size_t model_key_size() { return 1; }
  static Key max() {
    static Key max_key(std::numeric_limits<KeyType>::max());
    return max_key;
  }
  static Key min() {
    static Key min_key(std::numeric_limits<KeyType>::min());
    return min_key;
  }

  Key() : key(0) {}
  Key(KeyType key) : key(key) {}
  Key(const Key &other) { key = other.key; }
  Key &operator=(const Key &other) {
    key = other.key;
    return *this;
  }

  model_key_t to_model_key() const {
    model_key_t model_key;
    model_key[0] = key;
    return model_key;
  }

  friend bool operator<(const Key &l, const Key &r) { return l.key < r.key; }
  friend bool operator>(const Key &l, const Key &r) { return l.key > r.key; }
  friend bool operator>=(const Key &l, const Key &r) { return l.key >= r.key; }
  friend bool operator<=(const Key &l, const Key &r) { return l.key <= r.key; }
  friend bool operator==(const Key &l, const Key &r) { return l.key == r.key; }
  friend bool operator!=(const Key &l, const Key &r) { return l.key != r.key; }

  KeyType key;
} __attribute__((packed));

template <class KeyType, class SearchClass>
class XIndex : public Competitor<KeyType, SearchClass> {
 public:
  typedef Key<KeyType> index_key_t;
  typedef xindex::XIndex<index_key_t, uint64_t, false, SearchClass> xindex_t;

  XIndex(const std::vector<int>& params): error_bound(params[0]){}
  ~XIndex(){
    if (table){
      delete table;
    }
  }

  uint64_t Build(const std::vector<KeyValue<KeyType>>& data, size_t num_threads) {
    std::vector<index_key_t> keys;
    keys.reserve(data.size());

    std::vector<uint64_t> values;
    for (const KeyValue<KeyType>& kv : data) {
      keys.emplace_back(kv.key);
      values.push_back(kv.value);
    }

    num_workers_ = num_threads;

    uint64_t build_time =
        util::timing([&] { 
          if (num_threads > 1){
            table = new xindex_t(keys, values, num_workers_, 1, error_bound);
          }
          else{
            table = new xindex_t(keys, values, 1, 1, error_bound);
          }
    });

    return build_time;
  }

  size_t EqualityLookup(const KeyType& lookup_key, uint32_t thread_id) const {
    uint64_t guess;
    if (table->get(index_key_t(lookup_key), guess, thread_id)){
      return guess;
    }
    return util::OVERFLOW;
  }

  void Insert(const KeyValue<KeyType>& data, uint32_t thread_id) {
    table->put(index_key_t(data.key), data.value, thread_id);
  }

  std::string name() const { return "XIndex"; }

  std::size_t size() const { return table->size(); }

  bool applicable(bool unique, bool range_query, bool insert, bool multithread, const std::string& ops_filename) const {
    std::string name = SearchClass::name();
    return name != "LinearAVX" && name != "InterpolationSearch" && !std::is_same<KeyType, std::string>::value && !range_query && unique;
  }

  std::vector<std::string> variants() const { 
    std::vector<std::string> vec;
    vec.push_back(SearchClass::name());
    vec.push_back(std::to_string(error_bound));
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
  xindex_t* table = nullptr;
  size_t num_workers_;
  size_t error_bound;
};

};

#endif  // SOSDB_XINDEX_H
