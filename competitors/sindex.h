#ifndef TLI_SINDEX_H
#define TLI_SINDEX_H

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <vector>

#include "../util.h"
#include "base.h"
#include "sindex/sindex.h"
#include "sindex/sindex_impl.h"

namespace tli_sindex{

template <size_t len>
class StrKey {
  typedef std::array<double, len> model_key_t;

 public:
  static constexpr size_t model_key_size() { return len; }

  static StrKey max() {
    static StrKey max_key;
    memset(&max_key.buf, 255, len);
    return max_key;
  }
  static StrKey min() {
    static StrKey min_key;
    memset(&min_key.buf, 0, len);
    return min_key;
  }

  StrKey() { memset(&buf, 0, len); }
  StrKey(const std::string &s) {
    memset(&buf, 0, len);
    memcpy(&buf, s.data(), s.size());
  }
  StrKey(const StrKey &other) { memcpy(&buf, &other.buf, len); }
  StrKey &operator=(const StrKey &other) {
    memcpy(&buf, &other.buf, len);
    return *this;
  }

  model_key_t to_model_key() const {
    model_key_t model_key;
    for (size_t i = 0; i < len; i++) {
      model_key[i] = buf[i];
    }
    return model_key;
  }

  void get_model_key(size_t begin_f, size_t l, double *target) const {
    for (size_t i = 0; i < l; i++) {
      target[i] = buf[i + begin_f];
    }
  }

  bool less_than(const StrKey &other, size_t begin_i, size_t l) const {
    return memcmp(buf + begin_i, other.buf + begin_i, l) < 0;
  }

  friend bool operator<(const StrKey &l, const StrKey &r) {
    return memcmp(&l.buf, &r.buf, len) < 0;
  }
  friend bool operator>(const StrKey &l, const StrKey &r) {
    return memcmp(&l.buf, &r.buf, len) > 0;
  }
  friend bool operator>=(const StrKey &l, const StrKey &r) {
    return memcmp(&l.buf, &r.buf, len) >= 0;
  }
  friend bool operator<=(const StrKey &l, const StrKey &r) {
    return memcmp(&l.buf, &r.buf, len) <= 0;
  }
  friend bool operator==(const StrKey &l, const StrKey &r) {
    return memcmp(&l.buf, &r.buf, len) == 0;
  }
  friend bool operator!=(const StrKey &l, const StrKey &r) {
    return memcmp(&l.buf, &r.buf, len) != 0;
  }

  friend std::ostream &operator<<(std::ostream &os, const StrKey &key) {
    os << "key [" << std::hex;
    for (size_t i = 0; i < sizeof(StrKey); i++) {
      os << "0x" << key.buf[i] << " ";
    }
    os << "] (as byte)" << std::dec;
    return os;
  }

  uint8_t buf[len];
} __attribute__((packed));

template <class KeyType, class SearchClass>
class SIndex : public Competitor<KeyType, SearchClass> {
 public:
  typedef StrKey<128> index_key_t;
  typedef sindex::SIndex<index_key_t, uint64_t, false, SearchClass> sindex_t;

  SIndex(const std::vector<int>& params): et(params[0]), pt(params[1]){}
  ~SIndex(){
    if (table){
      delete table;
    }
  }

  uint64_t Build(const std::vector<KeyValue<KeyType>>& data, const size_t num_threads) {
    std::vector<index_key_t> keys;
    keys.reserve(data.size());
    std::vector<uint64_t> values;
    values.reserve(data.size());
    
    for (const KeyValue<KeyType>& kv : data) {
      keys.emplace_back(kv.key);
      values.push_back(kv.value);
    }

    num_workers_ = num_threads;

    uint64_t build_time =
        util::timing([&] { 
          if (num_threads > 1){
            table = new sindex_t(keys, values, num_workers_, (num_workers_ + 23) / 24, et, pt);
          }
          else{
            table = new sindex_t(keys, values, 1, 1, et, pt);
          }
    });

    return build_time;
  }

  size_t EqualityLookup(const KeyType lookup_key, uint32_t thread_id) const {
    uint64_t guess;
    if (table->get(index_key_t(lookup_key), guess, thread_id)){
      return guess;
    }
    return util::OVERFLOW;
  }

  uint64_t RangeQuery(const KeyType& lower_key, const KeyType& upper_key, uint32_t thread_id) const {
    std::vector<std::pair<index_key_t, uint64_t>> results;
    table->range_scan(index_key_t(lower_key), index_key_t(upper_key), results, thread_id);
    uint64_t result = 0;
    for (size_t i = 0; i < results.size(); ++ i){
      result += results[i].second;
    }
    return result;
  }

  void Insert(const KeyValue<KeyType>& data, uint32_t thread_id) {
    table->put(index_key_t(data.key), data.value, thread_id);
  }

  std::string name() const { return "SIndex"; }

  std::size_t size() const { return table->size(); }

  bool applicable(bool unique, bool range_query, bool insert, bool multithread, const std::string& ops_filename) const {
    std::string name = SearchClass::name();
    return name != "LinearAVX" && name != "InterpolationSearch" && std::is_same<KeyType, std::string>::value && unique;
  }

  std::vector<std::string> variants() const { 
    std::vector<std::string> vec;
    vec.push_back(SearchClass::name());
    vec.push_back(std::to_string(et));
    vec.push_back(std::to_string(pt));
    return vec;
  }

  struct alignas(CACHELINE_SIZE) SIndexParam{
    FGParam* param;
    void *(* func)(void *);
  };

  static void * myFunc(void * param){
    SIndexParam &thread_param = *(SIndexParam *)param;
    auto p = (*thread_param.func)((void *)thread_param.param);
    sindex::config.rcu_status[thread_param.param->thread_id].status =  std::numeric_limits<long long>::max();
    return p;
  }

  uint64_t runMultithread(void *(* func)(void *), FGParam *params) {
    pthread_t threads[num_workers_];
    SIndexParam sindexParams[num_workers_];

    for (size_t worker_i = 0; worker_i < num_workers_; worker_i++) {
      sindexParams[worker_i].func = func;
      sindexParams[worker_i].param = &params[worker_i];
      int ret = pthread_create(&threads[worker_i], nullptr, myFunc, (void *)&sindexParams[worker_i]);
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
  sindex_t* table = nullptr;
  size_t num_workers_;
  size_t et;
  size_t pt;
};

};

#endif  // TLI_SINDEX_H
