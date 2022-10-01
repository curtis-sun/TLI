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

#include "helper.h"
#include "sindex_buffer.h"
#include "sindex_group.h"
#include "sindex_model.h"
#include "sindex_root.h"
#include "sindex_util.h"

#if !defined(SINDEX_H)
#define SINDEX_H

namespace sindex {

template <class key_t, class val_t, bool seq = false>
class SIndex {
  typedef Group<key_t, val_t, seq> group_t;
  typedef Root<key_t, val_t, seq> root_t;
  typedef void iterator_t;

 public:
  SIndex(const std::vector<key_t> &keys, const std::vector<val_t> &vals,
         size_t worker_num, size_t bg_n);
  ~SIndex();

  inline bool get(const key_t &key, val_t &val, const uint32_t worker_id);
  inline bool put(const key_t &key, const val_t &val, const uint32_t worker_id);
  inline bool remove(const key_t &key, const uint32_t worker_id);
  inline size_t scan(const key_t &begin, const size_t n,
                     std::vector<std::pair<key_t, val_t>> &result,
                     const uint32_t worker_id);
  size_t range_scan(const key_t &begin, const key_t &end,
                    std::vector<std::pair<key_t, val_t>> &result,
                    const uint32_t worker_id);
 private:
  void start_bg();
  void terminate_bg();

  // this function should periodically check and perform structure updates
  static void *background(void *this_);

  root_t *volatile root = nullptr;
  pthread_t bg_master;
  size_t bg_num;
  volatile bool bg_running = true;
};

}  // namespace sindex

#endif  // SINDEX_H
