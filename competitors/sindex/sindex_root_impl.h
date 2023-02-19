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
#include <unordered_map>

#include "sindex_root.h"

#if !defined(SINDEX_ROOT_IMPL_H)
#define SINDEX_ROOT_IMPL_H

namespace sindex {

template <class key_t, class val_t, bool seq, class SearchClass>
Root<key_t, val_t, seq, SearchClass>::~Root() {}

template <class key_t, class val_t, bool seq, class SearchClass>
void Root<key_t, val_t, seq, SearchClass>::free_memory(){
  for (size_t group_i = 0; group_i < group_n; group_i++) {
    group_t *group = groups[group_i].second;
    while (group != nullptr) {
      group_t * old_group = group;
      group = group->next;
      old_group->free_data();
      old_group->free_buffer();
      old_group->free_buffer_temp();
      delete old_group;
    }
    groups[group_i].second = nullptr;
  }
}

template <class key_t, class val_t, bool seq, class SearchClass>
void Root<key_t, val_t, seq, SearchClass>::init(const std::vector<key_t> &keys,
                                   const std::vector<val_t> &vals) {
  INVARIANT(seq == false);

  std::vector<size_t> pivot_indexes;
  grouping_by_partial_key(keys, config.group_error_bound,
                          config.partial_len_bound, config.forward_step,
                          config.backward_step, config.group_min_size,
                          pivot_indexes);

  group_n = pivot_indexes.size();
  size_t record_n = keys.size();
  groups = std::make_unique<std::pair<key_t, group_t *volatile>[]>(group_n);

  double max_group_error = 0, avg_group_error = 0;
  double avg_prefix_len = 0, avg_feature_len = 0;
  size_t feature_begin_i = std::numeric_limits<uint16_t>::max(),
         max_feature_len = 0;

  for (size_t group_i = 0; group_i < group_n; group_i++) {
    size_t begin_i = pivot_indexes[group_i];
    size_t end_i =
        group_i + 1 == group_n ? record_n : pivot_indexes[group_i + 1];

    set_group_pivot(group_i, keys[begin_i]);
    group_t *group_ptr = new group_t();
    group_ptr->init(keys.begin() + begin_i, vals.begin() + begin_i,
                    end_i - begin_i);
    set_group_ptr(group_i, group_ptr);
    assert(group_ptr == get_group_ptr(group_i));

    avg_prefix_len += get_group_ptr(group_i)->prefix_len;
    avg_feature_len += get_group_ptr(group_i)->feature_len;
    feature_begin_i =
        std::min(feature_begin_i, (size_t)groups[group_i].second->prefix_len);
    max_feature_len =
        std::max(max_feature_len, (size_t)groups[group_i].second->feature_len);
    double err = get_group_ptr(group_i)->get_mean_error();
    max_group_error = std::max(max_group_error, err);
    avg_group_error += err;
  }

  // then decide # of 2nd stage model of root RMI
  adjust_root_model();

  DEBUG_THIS("------ Final SIndex Paramater: group_n="
             << group_n << ", avg_group_size=" << keys.size() / group_n
             << ", feature_begin_i=" << feature_begin_i
             << ", max_feature_len=" << max_feature_len);
  DEBUG_THIS("------ Final SIndex Errors: max_error="
             << max_group_error
             << ", avg_group_error=" << avg_group_error / group_n
             << ", avg_prefix_len=" << avg_prefix_len / group_n
             << ", avg_feature_len=" << avg_feature_len / group_n);
  DEBUG_THIS("------ Final SIndex Memory: sizeof(root)="
             << sizeof(*this) << ", sizeof(group_t)=" << sizeof(group_t)
             << ", sizeof(group_t::record_t)="
             << sizeof(typename group_t::record_t)
             << ", sizeof(group_t::wrapped_val_t)="
             << sizeof(typename group_t::wrapped_val_t));
}

/*
 * Root::put
 */
template <class key_t, class val_t, bool seq, class SearchClass>
inline result_t Root<key_t, val_t, seq, SearchClass>::put(const key_t &key, const val_t &val,
                                             const uint32_t worker_id) {
  return locate_group(key)->put(key, val, worker_id);
}

/*
 * Root::remove
 */
template <class key_t, class val_t, bool seq, class SearchClass>
inline result_t Root<key_t, val_t, seq, SearchClass>::remove(const key_t &key) {
  return locate_group(key)->remove(key);
}

template <class key_t, class val_t, bool seq, class SearchClass>
inline size_t Root<key_t, val_t, seq, SearchClass>::scan(
    const key_t &begin, const size_t n,
    std::vector<std::pair<key_t, val_t>> &result) {
  size_t remaining = n;
  result.clear();
  result.reserve(n);
  key_t next_begin = begin;
  key_t latest_group_pivot = key_t::min();  // for cross-slot chained groups

  int group_i;
  group_t *group = locate_group_pt2(begin, locate_group_pt1(begin, group_i));
  while (remaining && group_i < (int)group_n) {
    while (remaining && group &&
           group->pivot > latest_group_pivot /* avoid re-entry */) {
      size_t done = group->scan(next_begin, remaining, result);
      assert(done <= remaining);
      remaining -= done;
      latest_group_pivot = group->pivot;
      next_begin = key_t::min();  // though don't know the exact begin
      group = group->next;
    }
    group_i++;
    group = get_group_ptr(group_i);
  }

  return n - remaining;
}

template <class key_t, class val_t, bool seq, class SearchClass>
inline size_t Root<key_t, val_t, seq, SearchClass>::range_scan(
    const key_t &begin, const key_t &end,
    std::vector<std::pair<key_t, val_t>> &result) {
  bool is_begin = false, is_end = false;
  result.clear();
  key_t latest_group_pivot = key_t::min();

  int group_i;
  group_t *group = locate_group_pt2(begin, locate_group_pt1(begin, group_i));

  group->range_scan(begin, end, result);
  latest_group_pivot = group->pivot;
  group = group->next;
  while (true) {
    while (group && group->pivot > latest_group_pivot) {
      if (group->pivot > end){
        is_end = true;
        break;
      }
      is_begin |= (group->pivot > begin);

      group->range_scan((is_begin ? key_t::min() : begin), end, result);
      latest_group_pivot = group->pivot;
      group = group->next;
    }
    group_i++;
    if (is_end || group_i > (int)group_n - 1){
      break;
    }
    group = groups[group_i].second;
  }

  return 0;
}

template <class key_t, class val_t, bool seq, class SearchClass>
void *Root<key_t, val_t, seq, SearchClass>::do_adjustment(void *args) {
  std::atomic<bool> &started = ((BGInfo *)args)->started;
  std::atomic<bool> &finished = ((BGInfo *)args)->finished;
  size_t bg_i = (((BGInfo *)args)->bg_i);
  size_t bg_num = (((BGInfo *)args)->bg_n);
  volatile bool &running = ((BGInfo *)args)->running;

  while (running) {
    sleep(1);
    if (started) {
      started = false;

      // read the current root ptr and bg thread's responsible range
      Root &root = **(Root * volatile *)(((BGInfo *)args)->root_ptr);
      size_t begin_group_i = bg_i * root.group_n / bg_num;
      size_t end_group_i = bg_i == bg_num - 1
                               ? root.group_n
                               : (bg_i + 1) * root.group_n / bg_num;

      // iterate through the array, and do maintenance
      size_t m_split = 0, g_split = 0, m_merge = 0, g_merge = 0, compact = 0;
      size_t buf_size = 0, cnt = 0;
      for (size_t group_i = begin_group_i; group_i < end_group_i; group_i++) {
        if (group_i % 50000 == 0) {
          DEBUG_THIS("------ [structure update] doing group_i=" << group_i);
        }

        group_t *volatile *group = &(root.groups[group_i].second);
        while (*group != nullptr) {
// disable model split/merge
#if 0
          // check model split/merge
          bool should_split_group = false;
          bool might_merge_group = false;

          size_t max_trial_n =
              max_group_model_n;  // set this to avoid ping-pong effect
          for (size_t trial_i = 0; trial_i < max_trial_n; ++trial_i) {
            group_t *old_group = (*group);

            double mean_error;
            if (seq) {
              mean_error = old_group->mean_error_est();
            } else {
              mean_error = old_group->get_mean_error();
            }

            uint16_t model_n = old_group->model_n;
            if (mean_error > config.group_error_bound) {
              if (model_n != max_group_model_n) {
                // DEBUG_THIS("split model, err: " << mean_error
                //  << ", trial_i: " << trial_i);
                *group = old_group->split_model();
                memory_fence();
                rcu_barrier();
                if (seq) {
                  (*group)->enable_seq_insert_opt();
                }
                m_split++;
                delete old_group;
              } else {
                should_split_group = true;
                break;
              }
            } else if (mean_error < config.group_error_bound /
                                        config.group_error_tolerance) {
              if (model_n != 1) {
                // DEBUG_THIS("merge model, err: " << mean_error
                //  << ", trial_i: " << trial_i);
                *group = old_group->merge_model();
                memory_fence();
                rcu_barrier();
                if (seq) {
                  (*group)->enable_seq_insert_opt();
                }
                m_merge++;
                delete old_group;
              } else {
                might_merge_group = true;
                break;
              }
            } else {
              break;
            }
          }

          // prepare for group merge
          group_t *volatile *next_group = nullptr;
          if ((*group)->next) {
            next_group = &((*group)->next);
          } else if (group_i != end_group_i - 1 &&
                     root.get_group_ptr(group_i + 1)) {
            next_group = &(root.groups[group_i + 1].second);
          }
#endif

          // check for group split/merge, if not, do compaction
          size_t buffer_size = (*group)->buffer->size();
          buf_size += buffer_size;
          cnt++;
          group_t *old_group = (*group);

// disable group split/merge
#if 0
          if (should_split_group || buffer_size > config.buffer_size_bound) {
            // DEBUG_THIS("split group, buf_size: " << buffer_size);

            group_t *intermediate = old_group->split_group_pt1();
            *group = intermediate;  // create 2 new groups with freezed buffer
            memory_fence();
            rcu_barrier();  // make sure no one is inserting to buffer
            group_t *new_group = intermediate->split_group_pt2();  // now merge
            *group = new_group;
            memory_fence();
            rcu_barrier();  // make sure no one is using old/intermedia groups
            g_split++;
            new_group->compact_phase_2();
            new_group->next->compact_phase_2();
            memory_fence();
            rcu_barrier();  // make sure no one is accessing the old data
            old_group->free_data();  // intermidiates share the array and buffer
            old_group->free_buffer();  // so no free_xxx is needed
            delete old_group;
            delete intermediate->next;  // but deleting the metadata is needed
            delete intermediate;
            ((BGInfo *)args)->should_update_array = true;

            // skip next (the split new one), to avoid ping-pong split / merge
            group = &((*group)->next);
          } else if (might_merge_group &&
                  buffer_size < config.buffer_size_bound /
                               config.buffer_size_tolerance &&
                  next_group != nullptr) {
            if (seq == true) {
              COUT_VAR(group_i);
              COUT_VAR(begin_group_i);
              COUT_VAR(end_group_i);
              COUT_VAR(old_group);
              COUT_VAR(next_group);
            }

            // DEBUG_THIS("merge group, buf_size: " << buffer_size);

            group_t *old_next = (*next_group);
            group_t *new_group = old_group->merge_group(*old_next);
            *group = new_group;
            *next_group = new_group;  // first set 2 ptrs to a valid one
            memory_fence();  // make sure that no one is accessing old groups
            rcu_barrier();   // before nullify the old next
            *next_group = nullptr;  // then nullify the next
            g_merge++;
            new_group->compact_phase_2();
            memory_fence();
            rcu_barrier();  // make sure no one is accessing the old data
            old_group->free_data();
            old_group->free_buffer();
            old_next->free_data();
            old_next->free_buffer();
            delete old_group;
            delete old_next;
            ((BGInfo *)args)->should_update_array = true;
          } else
#endif
          if (buffer_size > config.buffer_compact_threshold) {
            group_t *new_group = old_group->compact_phase_1();
            *group = new_group;
            memory_fence();
            rcu_barrier();
            compact++;
            new_group->compact_phase_2();
            memory_fence();
            rcu_barrier();  // make sure no one is accessing the old data
            old_group->free_data();
            old_group->free_buffer();
            delete old_group;
          }

          // do next (in the chain)
          group = &((*group)->next);
        }
      }

      finished = true;
      DEBUG_THIS("------ [structure update] m_split_n: " << m_split);
      DEBUG_THIS("------ [structure update] g_split_n: " << g_split);
      DEBUG_THIS("------ [structure update] m_merge_n: " << m_merge);
      DEBUG_THIS("------ [structure update] g_merge_n: " << g_merge);
      DEBUG_THIS("------ [structure update] compact_n: " << compact);
      DEBUG_THIS("------ [structure update] buf_size/cnt: " << 1.0 * buf_size /
                                                                   cnt);
      DEBUG_THIS("------ [structure update] done with "
                 << begin_group_i << "-" << end_group_i << " (" << cnt << ")");
    }
  }
  return nullptr;
}

template <class key_t, class val_t, bool seq, class SearchClass>
unsigned long long Root<key_t, val_t, seq, SearchClass>::size() const{
  unsigned long long size = sizeof(*this);
  for (size_t group_i = 0; group_i < group_n; group_i ++){
    group_t *group = get_group_ptr(group_i);
    while (group != nullptr) {
      size += group->size();
      group = group->next;
    }
  }
  return size;
}

template <class key_t, class val_t, bool seq, class SearchClass>
Root<key_t, val_t, seq, SearchClass> *Root<key_t, val_t, seq, SearchClass>::create_new_root() {
  Root *new_root = new Root();

  size_t new_group_n = 0;
  for (size_t group_i = 0; group_i < group_n; group_i++) {
    group_t *group = get_group_ptr(group_i);
    while (group != nullptr) {
      new_group_n++;
      group = group->next;
    }
  }

  DEBUG_THIS("--- [root] update root array. old_group_n="
             << group_n << ", new_group_n=" << new_group_n);
  new_root->group_n = new_group_n;
  new_root->groups = std::make_unique<std::pair<key_t, group_t *volatile>[]>(
      new_root->group_n);

  size_t new_group_i = 0;
  for (size_t group_i = 0; group_i < group_n; group_i++) {
    group_t *group = get_group_ptr(group_i);
    while (group != nullptr) {
      new_root->set_group_pivot(new_group_i, group->pivot);
      new_root->set_group_ptr(new_group_i, group);
      group = group->next;
      new_group_i++;
    }
  }

  for (size_t group_i = 0; group_i < new_root->group_n - 1; group_i++) {
    assert(new_root->get_group_pivot(group_i) <
           new_root->get_group_pivot(group_i + 1));
  }

  new_root->adjust_root_model();

  return new_root;
}

template <class key_t, class val_t, bool seq, class SearchClass>
void Root<key_t, val_t, seq, SearchClass>::trim_root() {
  for (size_t group_i = 0; group_i < group_n; group_i++) {
    group_t *group = get_group_ptr(group_i);
    if (group_i != group_n - 1) {
      assert(group->next ? group->next == get_group_ptr(group_i + 1) : true);
    }
    group->next = nullptr;
  }
}

template <class key_t, class val_t, bool seq, class SearchClass>
inline result_t Root<key_t, val_t, seq, SearchClass>::get(const key_t &key, val_t &val) {
  return locate_group(key)->get(key, val);
}

template <class key_t, class val_t, bool seq, class SearchClass>
void Root<key_t, val_t, seq, SearchClass>::adjust_root_model() {
  train_piecewise_model();

  std::vector<double> errors(group_n);
  for (size_t group_i = 0; group_i < group_n; group_i++) {
    errors[group_i] = std::abs(
        (double)group_i - (double)predict(get_group_ptr(group_i)->pivot) + 1);
  }
  double mean_error =
      std::accumulate(errors.begin(), errors.end(), 0.0) / errors.size();

  DEBUG_THIS("------ Final SIndex Root Piecewise Model: model_n="
             << this->root_model_n << ", error=" << mean_error);
}

template <class key_t, class val_t, bool seq, class SearchClass>
inline size_t Root<key_t, val_t, seq, SearchClass>::predict(const key_t &key) {
  uint32_t m_i = 0;
  while (m_i < root_model_n - 1 && key >= model_pivots[m_i + 1]) {
    m_i++;
  }
  uint32_t p_len = models[m_i].p_len, f_len = models[m_i].f_len;
  double model_key[f_len];
  key.get_model_key(p_len, f_len, model_key);
  return model_predict(models[m_i].weights.data(), model_key, f_len);
}

template <class key_t, class val_t, bool seq, class SearchClass>
inline void Root<key_t, val_t, seq, SearchClass>::train_piecewise_model() {
  std::vector<size_t> indexes;
  std::vector<key_t> pivots(group_n);
  for (size_t g_i = 0; g_i < group_n; ++g_i) {
    pivots[g_i] = get_group_pivot(g_i);
  }
  grouping_by_partial_key(pivots, config.group_error_bound,
                          config.partial_len_bound, config.forward_step,
                          config.backward_step, config.group_min_size, indexes);

  if (indexes.size() > max_root_model_n) {
    size_t index_per_model = indexes.size() / max_root_model_n;
    std::vector<size_t> new_indexes;
    size_t trailing = indexes.size() - index_per_model * max_root_model_n;
    size_t start_i = 0;
    for (size_t i = 0; i < max_root_model_n; ++i) {
      new_indexes.push_back(indexes[start_i]);
      start_i += index_per_model;
      if (trailing > 0) {
        trailing--;
        start_i++;
      }
    }
    indexes.swap(new_indexes);
  }

  uint32_t model_n = indexes.size();
  INVARIANT(model_n <= max_root_model_n);
  this->root_model_n = model_n;
  DEBUG_THIS("----- Root model n after grouping: " << model_n);

  assert(indexes.size() <= model_n);
  uint32_t p_len = 0, f_len = 0;
  for (size_t m_i = 0; m_i < indexes.size(); ++m_i) {
    size_t b_i = indexes[m_i];
    size_t e_i = (m_i == indexes.size() - 1) ? group_n : indexes[m_i + 1];
    model_pivots[m_i] = get_group_ptr(b_i)->pivot;
    partial_key_len_of_pivots(b_i, e_i, p_len, f_len);
    size_t m_size = e_i - b_i;
    DEBUG_THIS("------ SIndex Root Model(" << m_i << "): size=" << m_size
                                           << ", p_len=" << p_len
                                           << ", f_len=" << f_len);
    std::vector<double> m_keys(m_size * f_len);
    std::vector<double *> m_key_ptrs(m_size);
    std::vector<size_t> ps(m_size);
    for (size_t k_i = 0; k_i < m_size; ++k_i) {
      get_group_ptr(k_i + b_i)->pivot.get_model_key(
          p_len, f_len, m_keys.data() + f_len * k_i);
      m_key_ptrs[k_i] = m_keys.data() + f_len * k_i;
      ps[k_i] = k_i + b_i;
    }

    for (size_t i = 0; i < key_t::model_key_size() + 1; ++i) {
      models[m_i].weights[i] = 0;
    }
    models[m_i].p_len = p_len;
    models[m_i].f_len = f_len;
    model_prepare(m_key_ptrs, ps, models[m_i].weights.data(), f_len);
  }
}

template <class key_t, class val_t, bool seq, class SearchClass>
inline void Root<key_t, val_t, seq, SearchClass>::partial_key_len_of_pivots(
    const size_t start_i, const size_t end_i, uint32_t &p_len,
    uint32_t &f_len) {
  assert(start_i < end_i);
  if (end_i == start_i + 1) {
    p_len = 0;
    f_len = 1;
    return;
  }
  p_len = common_prefix_length(
      0, (uint8_t *)&(get_group_ptr(start_i)->pivot), sizeof(key_t),
      (uint8_t *)&(get_group_ptr(start_i + 1)->pivot), sizeof(key_t));
  size_t max_adjacent_prefix = p_len;

  for (size_t k_i = start_i + 2; k_i < end_i; ++k_i) {
    p_len = common_prefix_length(0, (uint8_t *)&(get_group_ptr(k_i - 1)->pivot),
                                 p_len, (uint8_t *)&(get_group_ptr(k_i)->pivot),
                                 sizeof(key_t));
    size_t adjacent_prefix = common_prefix_length(
        p_len, (uint8_t *)&(get_group_ptr(k_i - 1)->pivot), sizeof(key_t),
        (uint8_t *)&(get_group_ptr(k_i)->pivot), sizeof(key_t));
    assert(adjacent_prefix <= sizeof(key_t) - p_len);
    if (adjacent_prefix < sizeof(key_t) - p_len) {
      max_adjacent_prefix =
          std::max(max_adjacent_prefix, p_len + adjacent_prefix);
    }
  }
  f_len = max_adjacent_prefix - p_len + 2;
}

/*
 * Root::locate_group
 */
template <class key_t, class val_t, bool seq, class SearchClass>
inline typename Root<key_t, val_t, seq, SearchClass>::group_t *
Root<key_t, val_t, seq, SearchClass>::locate_group(const key_t &key) {
  int group_i;  // unused
  group_t *head = locate_group_pt1(key, group_i);
  return locate_group_pt2(key, head);
}

template <class key_t, class val_t, bool seq, class SearchClass>
inline typename Root<key_t, val_t, seq, SearchClass>::group_t *
Root<key_t, val_t, seq, SearchClass>::locate_group_pt1(const key_t &key, int &group_i) {
  group_i = predict(key);
  group_i = group_i > (int)group_n - 1 ? group_n - 1 : group_i;
  group_i = group_i < 0 ? 0 : group_i;

  // exponential search
  long long end_group_i = SearchClass::upper_bound(groups.get(), groups.get() + group_n, key, groups.get() + group_i,
                           std::function<key_t(std::pair<key_t, group_t *volatile>*)>([&](std::pair<key_t, group_t *volatile>* it)->key_t{ return it->first; })) - groups.get() - 1;
  // int begin_group_i, end_group_i;
  // if (get_group_pivot(group_i) <= key) {
  //   size_t step = 1;
  //   begin_group_i = group_i;
  //   end_group_i = begin_group_i + step;
  //   while (end_group_i < (int)group_n && get_group_pivot(end_group_i) <= key) {
  //     step = step * 2;
  //     begin_group_i = end_group_i;
  //     end_group_i = begin_group_i + step;
  //   }  // after this while loop, end_group_i might be >= group_n
  //   if (end_group_i > (int)group_n - 1) {
  //     end_group_i = group_n - 1;
  //   }
  // } else {
  //   size_t step = 1;
  //   end_group_i = group_i;
  //   begin_group_i = end_group_i - step;
  //   while (begin_group_i >= 0 && get_group_pivot(begin_group_i) > key) {
  //     step = step * 2;
  //     end_group_i = begin_group_i;
  //     begin_group_i = end_group_i - step;
  //   }  // after this while loop, begin_group_i might be < 0
  //   if (begin_group_i < 0) {
  //     begin_group_i = -1;
  //   }
  // }

  // now group[begin].pivot <= key && group[end + 1].pivot > key
  // in loop body, the valid search range is actually [begin + 1, end]
  // (inclusive range), thus the +1 term in mid is a must
  // this algorithm produces index to the last element that is <= key
  // while (end_group_i != begin_group_i) {
  //   // the "+2" term actually should be a "+1" after "/2", this is due to the
  //   // rounding in c++ when the first operant of "/" operator is negative
  //   int mid = (end_group_i + begin_group_i + 2) / 2;
  //   if (get_group_pivot(mid) <= key) {
  //     begin_group_i = mid;
  //   } else {
  //     end_group_i = mid - 1;
  //   }
  // }
  // the result falls in [-1, group_n - 1]
  // now we ensure the pointer is not null
  group_i = end_group_i < 0 ? 0 : end_group_i;
  group_t *group = get_group_ptr(group_i);
  while (group_i > 0 && group == nullptr) {
    group_i--;
    group = get_group_ptr(group_i);
  }
  // however, we treat the pivot key of the 1st group as -inf, thus we return
  // 0 when the search result is -1
  assert(get_group_ptr(0) != nullptr);

  return group;
}

template <class key_t, class val_t, bool seq, class SearchClass>
inline typename Root<key_t, val_t, seq, SearchClass>::group_t *
Root<key_t, val_t, seq, SearchClass>::locate_group_pt2(const key_t &key, group_t *begin) {
  group_t *group = begin;
  group_t *next = group->next;
  while (next != nullptr && next->pivot <= key) {
    group = next;
    next = group->next;
  }
  return group;
}

template <class key_t, class val_t, bool seq, class SearchClass>
inline void Root<key_t, val_t, seq, SearchClass>::set_group_ptr(size_t group_i,
                                                   group_t *g_ptr) {
  groups[group_i].second = g_ptr;
}

template <class key_t, class val_t, bool seq, class SearchClass>
inline typename Root<key_t, val_t, seq, SearchClass>::group_t *
Root<key_t, val_t, seq, SearchClass>::get_group_ptr(size_t group_i) const {
  return groups[group_i].second;
}

template <class key_t, class val_t, bool seq, class SearchClass>
inline void Root<key_t, val_t, seq, SearchClass>::set_group_pivot(size_t group_i,
                                                     const key_t &key) {
  groups[group_i].first = key;
}

template <class key_t, class val_t, bool seq, class SearchClass>
inline key_t &Root<key_t, val_t, seq, SearchClass>::get_group_pivot(size_t group_i) const {
  return groups[group_i].first;
}

template <class key_t, class val_t, bool seq, class SearchClass>
inline void Root<key_t, val_t, seq, SearchClass>::grouping_by_partial_key(
    const std::vector<key_t> &keys, size_t et, size_t pt, size_t fstep,
    size_t bstep, size_t min_size, std::vector<size_t> &pivot_indexes) const {
  pivot_indexes.clear();
  size_t start_i = 0, end_i = 0;
  size_t common_p_len = 0, max_p_len = 0, f_len = 0;
  double group_error = 0;
  double avg_group_size = 0, avg_f_len = 0;
  std::unordered_map<size_t, size_t> common_p_history;
  std::unordered_map<size_t, size_t> max_p_history;

  // prepare model keys for training
  std::vector<double> model_keys(keys.size() * sizeof(key_t));
  for (size_t k_i = 0; k_i < keys.size(); ++k_i) {
    keys[k_i].get_model_key(0, sizeof(key_t),
                            model_keys.data() + sizeof(key_t) * k_i);
  }

  while (end_i < keys.size()) {
    common_p_len = 0;
    max_p_len = 0;
    f_len = 0;
    group_error = 0;
    common_p_history.clear();
    max_p_history.clear();

    while (f_len < pt && group_error < et) {
      size_t pre_end_i = end_i;
      end_i += fstep;
      if (end_i >= keys.size()) {
        DEBUG_THIS("[Grouping] reach end. last group size= "
                   << (keys.size() - start_i));
        break;
      }
      partial_key_len_by_step(keys, start_i, pre_end_i, end_i, common_p_len,
                              max_p_len, common_p_history, max_p_history);
      INVARIANT(common_p_len <= max_p_len);
      f_len = max_p_len - common_p_len + 1;
      if (f_len >= pt) break;
      // group_error =
      //     train_and_get_err(model_keys, start_i, end_i, common_p_len, f_len);
    }

    while (f_len > pt || group_error > et) {
      if (end_i >= keys.size()) {
        DEBUG_THIS("[Grouping] reach end. last group size= "
                   << (keys.size() - start_i));
        break;
      }
      if (end_i - start_i < min_size) {
        end_i = start_i + min_size;
        break;
      }
      end_i -= bstep;
      partial_key_len_by_step(keys, start_i, start_i, end_i, common_p_len,
                              max_p_len, common_p_history, max_p_history);
      INVARIANT(common_p_len <= max_p_len);
      f_len = max_p_len - common_p_len + 1;
      // group_error =
      //     train_and_get_err(model_keys, start_i, end_i, common_p_len, f_len);
    }

    assert(f_len <= pt || end_i - start_i == min_size);
    pivot_indexes.push_back(start_i);
    avg_group_size += (end_i - start_i);
    avg_f_len += f_len;
    start_i = end_i;

    if (pivot_indexes.size() % 1000 == 0) {
      DEBUG_THIS("[Grouping] current size="
                 << pivot_indexes.size()
                 << ", avg_group_size=" << avg_group_size / pivot_indexes.size()
                 << ", avg_f_len=" << avg_f_len / pivot_indexes.size()
                 << ", current_group_error=" << group_error);
    }
  }

  DEBUG_THIS("[Grouping] group number="
             << pivot_indexes.size()
             << ", avg_group_size=" << avg_group_size / pivot_indexes.size()
             << ", avg_f_len=" << avg_f_len / pivot_indexes.size());
}

template <class key_t, class val_t, bool seq, class SearchClass>
inline void Root<key_t, val_t, seq, SearchClass>::partial_key_len_by_step(
    const std::vector<key_t> &keys, const size_t start_i,
    const size_t step_start_i, const size_t step_end_i, size_t &common_p_len,
    size_t &max_p_len, std::unordered_map<size_t, size_t> &common_p_history,
    std::unordered_map<size_t, size_t> &max_p_history) const {
  assert(start_i < step_end_i);

  if (common_p_history.count(step_end_i) > 0) {
    INVARIANT(max_p_history.count(step_end_i) > 0);
    common_p_len = common_p_history[step_end_i];
    max_p_len = max_p_history[step_end_i];
    return;
  }

  size_t offset = 0;  // we set offset to 0 intentionally! if it is not the
                      // first batch of calculate partial key length, we need to
                      // take the last element of last batch into account.

  if (step_start_i == start_i) {
    common_p_history.clear();
    max_p_history.clear();
    common_p_len =
        common_prefix_length(0, (uint8_t *)&keys[step_start_i], sizeof(key_t),
                             (uint8_t *)&keys[step_start_i + 1], sizeof(key_t));
    max_p_len = common_p_len;
    common_p_history[step_start_i + 1] = common_p_len;
    max_p_history[step_start_i + 1] = max_p_len;
    offset = 2;
  }

  for (size_t k_i = step_start_i + offset; k_i < step_end_i; ++k_i) {
    common_p_len =
        common_prefix_length(0, (uint8_t *)&keys[k_i - 1], common_p_len,
                             (uint8_t *)&keys[k_i], sizeof(key_t));
    size_t adjacent_prefix = common_prefix_length(
        common_p_len, (uint8_t *)&keys[k_i - 1], sizeof(key_t),
        (uint8_t *)&keys[k_i], sizeof(key_t));
    assert(adjacent_prefix <= sizeof(key_t) - common_p_len);
    if (adjacent_prefix < sizeof(key_t) - common_p_len) {
      max_p_len = std::max(max_p_len, common_p_len + adjacent_prefix);
    }
    common_p_history[k_i] = common_p_len;
    max_p_history[k_i] = max_p_len;
  }
}

template <class key_t, class val_t, bool seq, class SearchClass>
inline double Root<key_t, val_t, seq, SearchClass>::train_and_get_err(
    std::vector<double> &model_keys, size_t start_i, size_t end_i, size_t p_len,
    size_t f_len) const {
  size_t key_n = end_i - start_i;
  assert(key_n > 2);
  std::vector<double *> model_key_ptrs(key_n);
  std::vector<size_t> positions(key_n);
  for (size_t k_i = 0; k_i < key_n; ++k_i) {
    model_key_ptrs[k_i] =
        model_keys.data() + (start_i + k_i) * sizeof(key_t) + p_len;
    positions[k_i] = k_i;
  }

  std::vector<double> weights(f_len + 1, 0);
  model_prepare(model_key_ptrs, positions, weights.data(), f_len);

  double errors = 0;
  for (size_t k_i = 0; k_i < key_n; ++k_i) {
    size_t predict_i =
        model_predict(weights.data(), model_key_ptrs[k_i], f_len);
    errors += predict_i >= k_i ? (predict_i - k_i) : (k_i - predict_i);
  }
  return errors / key_n;
}

}  // namespace sindex

#endif  // SINDEX_ROOT_IMPL_H
