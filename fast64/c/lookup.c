// < begin copyright > 
// Copyright Ryan Marcus 2020
// 
// This file is part of fast64.
// 
// fast64 is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// fast64 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with fast64.  If not, see <http://www.gnu.org/licenses/>.
// 
// < end copyright > 

#include <stddef.h>
#include "lookup.h"

#if AVX512 == 1
#include <immintrin.h>
int count_greater64(const uint64_t* ptr, const uint64_t query) {
  __m512i key = _mm512_set1_epi64(*(int64_t*)&query);
  __m512i page = _mm512_load_si512(ptr);
  __mmask8 res = _mm512_cmpgt_epu64_mask(key, page);
  return __builtin_popcount(res);
}

int count_greater_eq64(const uint64_t* ptr, const uint64_t query) {
  __m512i key = _mm512_set1_epi64(*(int64_t*)&query);
  __m512i page = _mm512_load_si512(ptr);
  __mmask8 res = _mm512_cmpge_epu64_mask(key, page);
  return __builtin_popcount(res);
}

int count_greater32(const uint32_t* ptr, const uint32_t query) {
  __m512i key = _mm512_set1_epi32(*(int32_t*)&query);
  __m512i page = _mm512_load_si512(ptr);
  __mmask16 res = _mm512_cmpgt_epu32_mask(key, page);
  return __builtin_popcount(res);
}

int count_greater_eq32(const uint32_t* ptr, const uint32_t query) {
  __m512i key = _mm512_set1_epi32(*(int32_t*)&query);
  __m512i page = _mm512_load_si512(ptr);
  __mmask16 res = _mm512_cmpge_epu32_mask(key, page);
  return __builtin_popcount(res);
}
#endif


#if VANILLA == 1
int count_greater64(const uint64_t* ptr, const uint64_t key) {
  int count = 0;
  for (unsigned int i = 0; i < 8; i++) {
    if (key > ptr[i]) count++;
    else break;
  }
  return count;
}

int count_greater_eq64(const uint64_t* ptr, const uint64_t key) {
  int count = 0;
  for (unsigned int i = 0; i < 8; i++) {
    if (key >= ptr[i]) count++;
    else break;
  }
  return count;
}

int count_greater32(const uint32_t* ptr, const uint32_t key) {
  int count = 0;
  for (unsigned int i = 0; i < 16; i++) {
    if (key > ptr[i]) count++;
    else break;
  }
  return count;
}

int count_greater_eq32(const uint32_t* ptr, const uint32_t key) {
  int count = 0;
  for (unsigned int i = 0; i < 16; i++) {
    if (key >= ptr[i]) count++;
    else break;
  }
  return count;
}
#endif


void fast_lookup64(const uint64_t** internal_pages, const uint64_t num_internal_pages,
                   const uint64_t* leaf_page,
                   const uint64_t query,
                   uint64_t* const out1, uint64_t* const out2) {
  
  // go down the tree
  uint64_t idx = 0;
  for (size_t i = 0; i < num_internal_pages; i++) {
    int greater = count_greater64(internal_pages[i] + idx, query);
    idx = internal_pages[i][idx + 8 + greater];
  }

  int gt_count = count_greater_eq64(leaf_page + idx, query);

  if (gt_count == 0) {
    *out1 = leaf_page[idx - 16 + 15];
    *out2 = leaf_page[idx + 8];
  } else if (gt_count == 8) {
    *out1 = leaf_page[idx+15];
    *out2 = leaf_page[idx+16+8];
  } else {
    *out1 = leaf_page[idx + 8 + (gt_count - 1)];
    *out2 = leaf_page[idx + 8 + (gt_count - 1) + 1];
  }
}

void fast_lookup32(const uint32_t** internal_pages, const uint32_t num_internal_pages,
                   const uint32_t* leaf_page,
                   const uint32_t query,
                   uint32_t* const out1, uint32_t* const out2) {

  // go down the tree
  uint64_t idx = 0;
  for (size_t i = 0; i < num_internal_pages; i++) {
    int greater = count_greater32(internal_pages[i] + idx, query);
    idx = internal_pages[i][idx + 16 + greater];
  }

  int gt_count = count_greater_eq32(leaf_page + idx, query);

  if (gt_count == 0) {
    *out1 = leaf_page[idx - 32 + 31];
    *out2 = leaf_page[idx + 16];
  } else if (gt_count == 16) {
    *out1 = leaf_page[idx+31];
    *out2 = leaf_page[idx+32+16];
  } else {
    *out1 = leaf_page[idx + 16 + (gt_count - 1)];
    *out2 = leaf_page[idx + 16 + (gt_count - 1) + 1];
  }
}
