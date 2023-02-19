/*
 * The code is part of the SIndex project.
 *
 *    Copyright (C) 2020 Institute of Parallel and Distributed Systems (IPADS),
 * Shanghai Jiao Tong University. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "sindex_buffer.h"
#include "sindex_model.h"
#include "sindex_util.h"

#if !defined(SINDEX_GROUP_H)
#define SINDEX_GROUP_H

namespace sindex {

template <class key_t, class val_t, bool seq, class SearchClass, size_t max_model_n = 4>
class alignas(CACHELINE_SIZE) Group {
  typedef AtomicVal<val_t> atomic_val_t;
  typedef atomic_val_t wrapped_val_t;
  typedef AltBtreeBuffer<key_t, val_t> buffer_t;
  typedef uint64_t version_t;
  typedef std::pair<key_t, wrapped_val_t> record_t;

  template <class key_tt, class val_tt, bool sequential, class sc>
  friend class SIndex;
  template <class key_tt, class val_tt, bool sequential, class sc>
  friend class Root;

  struct ArrayDataSource {
    ArrayDataSource(record_t *data, uint32_t array_size, uint32_t pos);
    void advance_to_next_valid();
    const key_t &get_key();
    const val_t &get_val();

    uint32_t array_size, pos;
    record_t *data;
    bool has_next;
    key_t next_key;
    val_t next_val;
  };

  struct ArrayRefSource {
    ArrayRefSource(record_t *data, uint32_t array_size);
    void advance_to_next_valid();
    const key_t &get_key();
    atomic_val_t &get_val();

    uint32_t array_size, pos;
    record_t *data;
    bool has_next;
    key_t next_key;
    atomic_val_t *next_val_ptr;
  };

 public:
  Group();
  ~Group();
  void init(const typename std::vector<key_t>::const_iterator &keys_begin,
            const typename std::vector<val_t>::const_iterator &vals_begin,
            uint32_t array_size);
  void init(const typename std::vector<key_t>::const_iterator &keys_begin,
            const typename std::vector<val_t>::const_iterator &vals_begin,
            uint32_t model_n, uint32_t array_size);

  inline result_t get(const key_t &key, val_t &val);
  inline result_t put(const key_t &key, const val_t &val,
                      const uint32_t worker_id);
  inline result_t remove(const key_t &key);
  inline size_t scan(const key_t &begin, const size_t n,
                     std::vector<std::pair<key_t, val_t>> &result);
  inline size_t range_scan(const key_t &begin, const key_t &end,
                           std::vector<std::pair<key_t, val_t>> &result);

  double mean_error_est() const;
  double get_mean_error() const;
  Group *split_model();
  Group *merge_model();
  Group *split_group_pt1();
  Group *split_group_pt2();
  Group *merge_group(Group &next_group);
  Group *compact_phase_1();
  void compact_phase_2();

  void free_data();
  void free_buffer();
  void free_buffer_temp();

  unsigned long long size() const;

 private:
  inline size_t locate_model(const key_t &key);

  inline bool get_from_array(const key_t &key, val_t &val);
  inline result_t update_to_array(const key_t &key, const val_t &val,
                                  const uint32_t worker_id);
  inline bool remove_from_array(const key_t &key);

  inline size_t get_pos_from_array(const key_t &key);
  inline size_t lower_bound_from_array(const key_t &key);
  inline size_t binary_search_key(const key_t &key, size_t pos_hint,
                                  size_t search_begin, size_t search_end);
  inline size_t exponential_search_key(const key_t &key, size_t pos_hint) const;
  inline size_t exponential_search_key(record_t *const data,
                                       uint32_t array_size, const key_t &key,
                                       size_t pos_hint) const;
  inline size_t exponential_search_key_no_predict(record_t *const data,
                                       uint32_t array_size, const key_t &key,
                                       size_t pos_hint) const;
  inline size_t exponential_search_complete_key(record_t *const data,
                                       uint32_t array_size, const key_t &key,
                                       size_t pos_hint) const;                            

  inline bool get_from_buffer(const key_t &key, val_t &val, buffer_t *buffer);
  inline bool update_to_buffer(const key_t &key, const val_t &val,
                               buffer_t *buffer);
  inline void insert_to_buffer(const key_t &key, const val_t &val,
                               buffer_t *buffer);
  inline bool remove_from_buffer(const key_t &key, buffer_t *buffer);

  void init_models(uint32_t model_n);
  void init_models(uint32_t model_n, size_t p_len, size_t f_len);
  void init_feature_length();
  inline double train_model(size_t model_i, size_t begin, size_t end);

  inline void merge_refs(record_t *&new_data, uint32_t &new_array_size,
                         int32_t &new_capacity) const;
  inline void merge_refs_n_split(record_t *&new_data_1,
                                 uint32_t &new_array_size_1,
                                 int32_t &new_capacity_1, record_t *&new_data_2,
                                 uint32_t &new_array_size_2,
                                 int32_t &new_capacity_2,
                                 const key_t &key) const;
  inline void merge_refs_with(const Group &next_group, record_t *&new_data,
                              uint32_t &new_array_size,
                              int32_t &new_capacity) const;
  inline void merge_refs_internal(record_t *new_data,
                                  uint32_t &new_array_size) const;
  inline size_t scan_2_way(const key_t &begin, const size_t n, const key_t &end,
                           std::vector<std::pair<key_t, val_t>> &result);
  inline size_t scan_3_way(const key_t &begin, const size_t n, const key_t &end,
                           std::vector<std::pair<key_t, val_t>> &result);
  void seq_lock();
  void seq_unlock();
  inline void enable_seq_insert_opt();
  inline void disable_seq_insert_opt();

  inline double *get_model(size_t model_i) const;
  inline const uint8_t *get_model_pivot(size_t model_i) const;
  inline void set_model_pivot(size_t model_i, const key_t &key);
  inline void get_model_error(size_t model_i, int &error_max,
                              int &error_min) const;
  inline void set_model_error(size_t model_i, int error_max, int error_min);

  inline void prepare_last(size_t model_i,
                           const std::vector<double *> &model_keys,
                           const std::vector<size_t> &positions);
  inline size_t get_error_bound(size_t model_i,
                                const std::vector<double *> &model_key_ptrs,
                                const std::vector<size_t> &positions) const;
  inline void predict_last(size_t model_i, const key_t &key, size_t &pos,
                           int &error_min, int &error_max) const;
  inline size_t predict(size_t model_i, const key_t &key) const;
  inline size_t predict(size_t model_i, const double *model_key) const;
  inline bool key_less_than(const uint8_t *k1, const uint8_t *k2,
                            size_t len) const;

  key_t pivot;
  // make array_size atomic because we don't want to acquire lock during `get`.
  // it is okay to obtain a stale (smaller) array_size during `get`.
  uint32_t array_size;              // 4B
  uint8_t model_n = 0;              // 1B
  uint8_t prefix_len = 0;           // 1B
  uint8_t feature_len = 0;          // 1B
  bool buf_frozen = false;          // 1B
  record_t *data = nullptr;         // 8B
  Group *next = nullptr;            // 8B
  buffer_t *buffer = nullptr;       // 8B
  buffer_t *buffer_temp = nullptr;  // 8B
  // used for sequential insertion
  int32_t capacity = 0;         // 4B
  uint16_t pos_last_pivot = 0;  // 2B
  volatile uint8_t lock = 0;    // 1B
  // model data
  std::array<uint8_t,
             (max_model_n - 1) * sizeof(key_t) +
                 max_model_n * sizeof(double) * (key_t::model_key_size() + 1)>
      model_info;
};

}  // namespace sindex

#endif  // SINDEX_GROUP_H
