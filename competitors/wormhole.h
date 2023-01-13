#pragma once

#include <type_traits>

#include "base.h"
#include "wormhole/lib.h"
#include "wormhole/kv.h"
#include "wormhole/wh.h"

static struct kv* kv_dup_in_count(struct kv* kv, void* priv) {
  int64_t current_usage = *((int64_t*)priv);
  *((int64_t*)priv) = current_usage + kv_size(kv);
  return kv_dup(kv);
}

static struct kv* kv_dup_out_count(struct kv* kv, struct kv* out) {
  return kv_dup2(kv, out);
}

static void kv_free_count(struct kv* kv, void* priv) {
  int64_t current_usage = *((int64_t*)priv);
  *((int64_t*)priv) = current_usage - kv_size(kv);
  free(kv);
}

template <class KeyType>
class Wormhole : public Base<KeyType> {
 public:
  Wormhole(const std::vector<int>& params){}
  ~Wormhole(){
    if (index){
      for (size_t i = 0; i < num_threads_; i++) {
        free(in[i]);
        free(out[i]);
        wormhole_unref(refs[i].instance);
      }
      delete[] refs;
      delete[] in;
      delete[] out;
      wormhole_destroy(index);
    }
  }
  uint64_t Build(const std::vector<KeyValue<KeyType>>& data, const size_t num_threads) {
    std::vector<std::string> keys;
    keys.reserve(data.size());
    for (const KeyValue<KeyType>& kv : data) {
      keys.push_back(util::convertToString(kv.key));
    }

    kvmap_mm allocator = (kvmap_mm){kv_dup_in_count, kv_dup_out_count,
                                    kv_free_count, (void*)&usage_};
    num_threads_ = num_threads;
    refs = new struct wormref_align[num_threads];
    in = new struct kv*[num_threads];
    out = new struct kv*[num_threads];
    for (size_t i = 0; i < num_threads; ++ i){
      in[i] = static_cast<struct kv*>(malloc(sizeof(struct kv) + 1024 + sizeof(uint64_t)));
      out[i] = static_cast<struct kv*>(malloc(sizeof(struct kv) + 1024 + sizeof(uint64_t)));
    }

    uint64_t timing = util::timing([&] {
      index = wormhole_create(&allocator);
      struct wormref * ref = wormhole_ref(index);
      for (size_t i = 0; i < data.size(); i++) {
        kv_refill(in[0], keys[i].c_str(), keys[i].length(), reinterpret_cast<const char* const>(&data[i].value), sizeof(uint64_t));
        wormhole_put(ref, in[0]);
      }
      wormhole_unref(ref);
    });

    for (size_t i = 0; i < num_threads; ++ i){
      refs[i].instance = whsafe_ref(index);
    }

    return timing;
  }

  size_t EqualityLookup(const KeyType& lookup_key, uint32_t thread_id) const {
    std::string key = util::convertToString(lookup_key);
    kv_refill(in[thread_id], key.c_str(), key.length(), NULL, 0);
    kref kref = kv_kref(in[thread_id]);
    if (whsafe_get(refs[thread_id].instance, &kref, out[thread_id])){
      auto result = *(uint64_t*)kv_vptr(out[thread_id]);
      return result;
    }
    return util::OVERFLOW;
  }

  uint64_t RangeQuery(const KeyType lower_key, const KeyType upper_key, uint32_t thread_id) const {
    auto iter = wormhole_iter_create(refs[thread_id].instance);
    std::string lkey = util::convertToString(lower_key), ukey = util::convertToString(upper_key);
    kv_refill(in[thread_id], lkey.c_str(), lkey.length(), NULL, 0);
    kref kref = kv_kref(in[thread_id]);
    whsafe_iter_seek(iter, &kref);

    uint64_t result = 0;
    while (wormhole_iter_next(iter, out[thread_id])){
      if (std::string((char*)kv_kptr(out[thread_id]), out[thread_id]->klen) > ukey){
        break;
      }
      result += *(uint64_t*)kv_vptr(out[thread_id]);
    }
    
    whsafe_iter_destroy(iter);
    return result;
  }
  
  void Insert(const KeyValue<KeyType>& data, uint32_t thread_id) {
    std::string key = util::convertToString(data.key);
    kv_refill(in[thread_id], key.c_str(), key.length(), reinterpret_cast<const char* const>(&data.value), sizeof(uint64_t));
    whsafe_put(refs[thread_id].instance, in[thread_id]);
  }

  std::string name() const { return "Wormhole"; }

  std::size_t size() const {
    // return used memory in bytes
    if (usage_ < 0) {
      util::fail("Wormhole memory usage was negative!");
    }
    return usage_;
  }

  static void * background(void * param) {
    WormholeParam &thread_param = *(WormholeParam *)param;
    thread_fork_join(thread_param.num_threads, thread_param.func, true, (void **)thread_param.params);
    return nullptr;
  }

  uint64_t runMultithread(void *(* func)(void *), FGParam *params) {
    FGParam** params_ = new FGParam*[num_threads_];
    for (size_t worker_i = 0; worker_i < num_threads_; worker_i++) {
      params_[worker_i] = &params[worker_i];
    }
    pthread_t thread;
    WormholeParam param;
    param.func = func;
    param.params = params_;
    param.num_threads = num_threads_;

    int ret = pthread_create(&thread, nullptr, background,
                              (void *)&param);
    if (ret) {
      std::cout << "Error: " << ret << std::endl;
    }

    std::cout << "[micro] prepare data ...\n";
    while (util::ready_threads < num_threads_) sleep(1);
    util::running = true;

    uint64_t timing = util::timing([&] {
      while (util::running)
        ;
    });

    void *status;
    int rc = pthread_join(thread, &status);
    if (rc) {
      std::cout << "Error:unable to join, " << rc << std::endl;
    }

    delete[] params_;
    return timing;
  }

  bool applicable(bool unique, bool range_query, bool insert, bool multithread, const std::string& ops_filename) {
    // only supports unique keys.
    return unique;
  }

 private:
  struct alignas(CACHELINE_SIZE) wormref_align {
    struct wormref *instance;
  };
  struct alignas(CACHELINE_SIZE) WormholeParam{
    void *(* func)(void *); 
    FGParam** params;
    size_t num_threads;
  };

  struct wormhole* index = NULL;
  struct wormref_align* refs = NULL;
  int64_t usage_ = 0;
  size_t num_threads_;
  struct kv ** in = NULL;
  struct kv ** out = NULL;
};
