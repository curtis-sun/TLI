#pragma once
#include "linear_search.h"
#include "../util.h"

#define SHUF(i0, i1, i2, i3) ((i0) + (i1) * 4 + (i2) * 16 + (i3) * 64)

template<typename Iterator, bool direction>
Iterator static forceinline scan_avx(std::function<Iterator(Iterator, Iterator, const uint32_t&, 
                            Iterator, std::function<uint32_t(Iterator)>, std::function<bool(const uint32_t&, const uint32_t&)>)> func_search,
                  std::function<__m256i(__m256i, __m256i)> func_cmp, 
                  Iterator move_it, Iterator end, const uint32_t& lookup_key,
                  std::function<uint32_t(Iterator)> at,
                  std::function<bool(const uint32_t&, const uint32_t&)> less){
  size_t n, i = 32;
  if constexpr (direction){
    n = std::distance(move_it, end);
  }
  else{
    n = std::distance(end, move_it);
  }

  if (n >= 32) {
    __m256i vkey = _mm256_set1_epi32(lookup_key);
    uint32_t res;
    while(i <= n) {
      if constexpr (!direction){
        std::advance(move_it, -8);
      }
      __m256i cmp0 = func_cmp(_mm256_loadu_si256((__m256i *)(&*move_it)), vkey);
      if constexpr (direction){
        std::advance(move_it, 8);
      }
      else{
        std::advance(move_it, -8);
      }                                                      
      __m256i cmp1 = func_cmp(_mm256_loadu_si256((__m256i *)(&*move_it)), vkey);
      if constexpr (direction){
        std::advance(move_it, 8);
      }
      else{
        std::advance(move_it, -8);
      }
      __m256i cmp2 = func_cmp(_mm256_loadu_si256((__m256i *)(&*move_it)), vkey);
      if constexpr (direction){
        std::advance(move_it, 8);
      }
      else{
        std::advance(move_it, -8);
      }
      __m256i cmp3 = func_cmp(_mm256_loadu_si256((__m256i *)(&*move_it)), vkey);
      if constexpr (direction){
        std::advance(move_it, 8); 
      }
      __m256i pack01 = _mm256_packs_epi32(cmp0, cmp1);
      __m256i pack23 = _mm256_packs_epi32(cmp2, cmp3);
      __m256i pack0123 = _mm256_packs_epi16(pack01, pack23);
      res = (uint32_t)_mm256_movemask_epi8(pack0123);
      if (res != (uint32_t)0xffffffff){
        break;
      }   
      i += 32;
    }

    if (i <= n){
      int delta = 32 - __builtin_popcount(res);
      if constexpr (direction){
        std::advance(move_it, -delta);
      }
      else{
        std::advance(move_it, delta);
      }
      return move_it;
    }
  }

  if constexpr (direction){
    return func_search(move_it, end, lookup_key, move_it, at, less);
  }
  else{
    return func_search(end, move_it, lookup_key, move_it, at, less);
  }
}

template<typename Iterator, bool direction>
Iterator static forceinline scan_avx(std::function<Iterator(Iterator, Iterator, const uint64_t&, 
                            Iterator, std::function<uint64_t(Iterator)>, std::function<bool(const uint64_t&, const uint64_t&)>)> func_search,
                  std::function<__m256i(__m256i, __m256i)> func_cmp, 
                  Iterator move_it, Iterator end, const uint64_t& lookup_key,
                  std::function<uint64_t(Iterator)> at,
                  std::function<bool(const uint64_t&, const uint64_t&)> less){
  size_t n, i = 16;
  if constexpr (direction){
    n = std::distance(move_it, end);
  }
  else{
    n = std::distance(end, move_it);
  }

  if (n >= 16){
    __m256i vkey = _mm256_set1_epi64x(lookup_key);
    uint32_t res;
    while(i <= n) {
      if constexpr (!direction){
        std::advance(move_it, -4);
      }
      __m256i cmp0 = func_cmp(_mm256_loadu_si256((__m256i *)(&*move_it)), vkey);
      if constexpr (direction){
        std::advance(move_it, 4);
      }
      else{
        std::advance(move_it, -4);
      }                                                      
      __m256i cmp1 = func_cmp(_mm256_loadu_si256((__m256i *)(&*move_it)), vkey);
      if constexpr (direction){
        std::advance(move_it, 4);
      }
      else{
        std::advance(move_it, -4);
      }
      __m256i cmp2 = func_cmp(_mm256_loadu_si256((__m256i *)(&*move_it)), vkey);
      if constexpr (direction){
        std::advance(move_it, 4);
      }
      else{
        std::advance(move_it, -4);
      }
      __m256i cmp3 = func_cmp(_mm256_loadu_si256((__m256i *)(&*move_it)), vkey);
      if constexpr (direction){
        std::advance(move_it, 4); 
      }
      __m256i pack01 = _mm256_packs_epi32(cmp0, cmp1);
      __m256i pack23 = _mm256_packs_epi32(cmp2, cmp3);
      __m256i pack0123 = _mm256_packs_epi16(pack01, pack23);
      res = (uint32_t)_mm256_movemask_epi8(pack0123);
      if (res != (uint32_t)0xffffffff){
        break;
      }   
      i += 16;
    }

    if (i <= n){
      int delta = 16 - (__builtin_popcount(res) >> 1);
      if constexpr (direction){
        std::advance(move_it, -delta);
      }
      else{
        std::advance(move_it, delta);
      }
      return move_it;
    }
  }

  if constexpr (direction){
    return func_search(move_it, end, lookup_key, move_it, at, less);
  }
  else{
    return func_search(end, move_it, lookup_key, move_it, at, less);
  }
}

template<typename KeyType, int record>
class LinearAVX: public Search<record>{};

template<int record>
class LinearAVX<uint32_t, record>: public Search<record> {
 public:
  template<typename Iterator>
  static forceinline Iterator lower_bound(
    Iterator first, Iterator last,
		const uint32_t& lookup_key, Iterator start,
    std::function<uint32_t(Iterator)> at = [](Iterator it)->uint32_t{
      return static_cast<uint32_t>(*it);
    },
    std::function<bool(const uint32_t&, const uint32_t&)> less = [](const uint32_t& key1, const uint32_t& key2)->bool{
      return key1 < key2;
    }) {
      record_start();
      if (first == last) {
        record_end(first, first);
        return first;
      }

      Iterator it;
      if (start != last && at(start) < lookup_key){
        Iterator mid = start;
        ++mid;
        it = scan_avx<Iterator, true>(LinearSearch<0>::lower_bound<Iterator, uint32_t>, _mm256_cmplt_epu32, mid, last, lookup_key, at, less);
      }
      else{
        it = scan_avx<Iterator, false>(LinearSearch<0>::lower_bound<Iterator, uint32_t>, _mm256_cmpge_epu32, start, first, lookup_key, at, less);
      }

      record_end(start, it);
      return it;
    }

  template<typename Iterator>
  static forceinline Iterator upper_bound(
    Iterator first, Iterator last,
		const uint32_t& lookup_key, Iterator start, 
    std::function<uint32_t(Iterator)> at = [](Iterator it)->uint32_t{
      return static_cast<uint32_t>(*it);
    },
    std::function<bool(const uint32_t&, const uint32_t&)> less = [](const uint32_t& key1, const uint32_t& key2)->bool{
      return key1 < key2;
    }) {
      record_start();
      if (first == last) {
        record_end(first, first);
        return first;
      }

      Iterator it;
      if (start == last || lookup_key < at(start)){
        it = scan_avx<Iterator, false>(LinearSearch<0>::upper_bound<Iterator, uint32_t>, _mm256_cmpgt_epu32, start, first, lookup_key, at, less);
      }
      else{
        Iterator mid = start;
        ++mid;
        it = scan_avx<Iterator, true>(LinearSearch<0>::upper_bound<Iterator, uint32_t>, _mm256_cmple_epu32, mid, last, lookup_key, at, less);
      }

      record_end(start, it);
      return it;
    }

  static std::string name() {
      return "LinearAVX";
  }
};

template<int record>
class LinearAVX<uint64_t, record>: public Search<record> {
 public:
  template<typename Iterator>
  static forceinline Iterator lower_bound(
    Iterator first, Iterator last,
		const uint64_t& lookup_key, Iterator start,
    std::function<uint64_t(Iterator)> at = [](Iterator it)->uint64_t{
      return static_cast<uint64_t>(*it);
    },
    std::function<bool(const uint64_t&, const uint64_t&)> less = [](const uint64_t& key1, const uint64_t& key2)->bool{
      return key1 < key2;
    }) {
      record_start();
      if (first == last) {
        record_end(first, first);
        return first;
      }

      Iterator it;
      if (start != last && at(start) < lookup_key){
        Iterator mid = start;
        ++mid;
        it = scan_avx<Iterator, true>(LinearSearch<0>::lower_bound<Iterator, uint64_t>, _mm256_cmplt_epu64, mid, last, lookup_key, at, less);
      }
      else{
        it = scan_avx<Iterator, false>(LinearSearch<0>::lower_bound<Iterator, uint64_t>, _mm256_cmpge_epu64, start, first, lookup_key, at, less);
      }

      record_end(start, it);
      return it;
    }

  template<typename Iterator>
  static forceinline Iterator upper_bound(
    Iterator first, Iterator last,
		const uint64_t& lookup_key, Iterator start, 
    std::function<uint64_t(Iterator)> at = [](Iterator it)->uint64_t{
      return static_cast<uint64_t>(*it);
    },
    std::function<bool(const uint64_t&, const uint64_t&)> less = [](const uint64_t& key1, const uint64_t& key2)->bool{
      return key1 < key2;
    }) {
      record_start();
      if (first == last) {
        record_end(first, first);
        return first;
      }

      Iterator it;
      if (start == last || lookup_key < at(start)){
        it = scan_avx<Iterator, false>(LinearSearch<0>::upper_bound<Iterator, uint64_t>, _mm256_cmpgt_epu64, start, first, lookup_key, at, less);
      }
      else{
        Iterator mid = start;
        ++mid;
        it = scan_avx<Iterator, true>(LinearSearch<0>::upper_bound<Iterator, uint64_t>, _mm256_cmple_epu64, mid, last, lookup_key, at, less);
      }

      record_end(start, it);
      return it;
    }

  static std::string name() {
      return "LinearAVX";
  }
};
