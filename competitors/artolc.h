#pragma once

#include "./ARTSynchronized/OptimisticLockCoupling/Tree.h"
#include "./ARTSynchronized/OptimisticLockCoupling/Tree.cpp"
#include "tbb/tbb.h"
#include "base.h"

template <class KeyType>
void convert2Key(const KeyType& org_key, Key &key) {
    if constexpr (std::is_same<KeyType, uint32_t>::value){
        key.setKeyLen(sizeof(org_key));
        reinterpret_cast<uint32_t *>(&key[0])[0] = __builtin_bswap32(org_key);
    }
    else if constexpr (std::is_same<KeyType, uint64_t>::value){
        key.setKeyLen(sizeof(org_key));
        reinterpret_cast<uint64_t *>(&key[0])[0] = __builtin_bswap64(org_key);
    }
    else{
        util::fail("Undefined key type.");
    }
}

template <>
void convert2Key(const std::string& org_key, Key &key) {
    key.setKeyLen(org_key.length() + 1);
    memcpy(&key[0], org_key.c_str(), org_key.length());
    key[org_key.length()] = 0;
}

template <class KeyType>
class ARTOLC : public Base<KeyType> {

 public:
  ARTOLC(const std::vector<int>& params){
    tree_ = new ART_OLC::Tree(this->loadKey, this->deleteKey);
  }
  ~ARTOLC(){
    delete tree_;
  }
  uint64_t Build(const std::vector<KeyValue<KeyType>>& data, const size_t num_threads) {
    num_workers_ = num_threads;
    return util::timing( [&] { 
        auto t = tree_->getThreadInfo();
        for (uint64_t i = 0; i < data.size(); i ++) {
            Element<KeyType>* e = new Element<KeyType>(data[i].key, data[i].value);
            Key key;
            convert2Key(data[i].key, key);
            tree_->insert(key, uint64_t(e) >> 1, t);
        } 
    });
  }

  size_t EqualityLookup(const KeyType lookup_key, uint32_t thread_id) const {
    auto t = tree_->getThreadInfo();
    Key key;
    convert2Key(lookup_key, key);
    auto idx = tree_->lookup(key, t);
    if (idx == util::OVERFLOW){
        return util::NOT_FOUND;
    }
    return reinterpret_cast<Element<KeyType>*>(idx << 1)->value;
  }

  uint64_t RangeQuery(const KeyType lower_key, const KeyType upper_key, uint32_t thread_id) const {
    auto t = tree_->getThreadInfo();
    Key lkey, ukey, continueKey;
    convert2Key(lower_key, lkey);
    convert2Key(upper_key, ukey);
    uint64_t result[200];
    std::size_t resultCount;
    tree_->lookupRange(lkey, ukey, continueKey, result, std::size_t(-1), resultCount, t);
    uint64_t sum = 0;
    for (std::size_t i = 0; i < resultCount; i ++){
        sum += reinterpret_cast<Element<KeyType>*>(result[i] << 1)->value;
    }
    return sum;
  }

  void Insert(const KeyValue<KeyType>& data, uint32_t thread_id) {
    auto t = tree_->getThreadInfo();
    Key key;
    convert2Key(data.key, key);
    Element<KeyType>* e = new Element<KeyType>(data.key, data.value);
    tree_->insert(key, uint64_t(e) >> 1, t);
  }

  std::string name() const { return "ARTOLC"; }

  std::size_t size() const { 
    return tree_->size();
  }

  bool applicable(bool unique, bool range_query, bool insert, bool multithread, const std::string& ops_filename) const {
    return unique;
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

  static void loadKey(uint64_t tid, Key &key) {
    convert2Key(reinterpret_cast<Element<KeyType>*>(tid << 1)->key, key);
  }

  static void deleteKey(uint64_t tid) {
    delete reinterpret_cast<Element<KeyType>*>(tid << 1);
  }

 private:
  ART_OLC::Tree* tree_;
  size_t num_workers_;
};
