#ifndef __UTIL_H__
#define __UTIL_H__

#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdio>
#include <stdlib.h>
#include <vector>
#include <cmath>
#include <climits>
#include <immintrin.h>
#include <cassert>
#include <random>
#include <memory>
#include <array>
#include <time.h>
#include <unistd.h>
#include <atomic>
#include <getopt.h>
#include <unistd.h>
#include <algorithm>
#include <mutex>


#define NS_PER_S 1000000000.0
#define TIMER_DECLARE(n) struct timespec b##n,e##n
#define TIMER_BEGIN(n) clock_gettime(CLOCK_MONOTONIC, &b##n)
#define TIMER_END_NS(n,t) clock_gettime(CLOCK_MONOTONIC, &e##n); \
    (t)=(e##n.tv_sec-b##n.tv_sec)*NS_PER_S+(e##n.tv_nsec-b##n.tv_nsec)
#define TIMER_END_S(n,t) clock_gettime(CLOCK_MONOTONIC, &e##n); \
    (t)=(e##n.tv_sec-b##n.tv_sec)+(e##n.tv_nsec-b##n.tv_nsec)/NS_PER_S

#define COUT_THIS(this) std::cout << this << std::endl;
#define COUT_VAR(this) std::cout << #this << ": " << this << std::endl;
#define COUT_POS() COUT_THIS("at " << __FILE__ << ":" << __LINE__)
#define COUT_N_EXIT(msg) \
  COUT_THIS(msg);        \
  COUT_POS();            \
  abort();
#define INVARIANT(cond)            \
  if (!(cond)) {                   \
    COUT_THIS(#cond << " failed"); \
    COUT_POS();                    \
    abort();                       \
  }

#if defined(NDEBUGGING)
#define DEBUG_THIS(this)
#else
#define DEBUG_THIS(this) std::cerr << this << std::endl
#endif

#define SUB_EPS(x, epsilon) ((x) <= (epsilon) ? 0 : ((x) - (epsilon)))
#define ADD_EPS(x, epsilon, size) ((x) + (epsilon) + 2 >= (size) ? (size-1) : (x) + (epsilon) + 2)

struct ApproxPos {
    size_t pos; ///< The approximate position of the key.
    size_t lo;  ///< The lower bound of the range.
    size_t hi;  ///< The upper bound of the range.
};

enum class Result { ok, failed, retry, retrain };
typedef Result result_t;

#define CACHELINE_SIZE (1 << 6)

// ==================== memory fence =========================
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

inline void memory_fence() { asm volatile("mfence" : : : "memory"); }

/** @brief Compiler fence.
 * Prevents reordering of loads and stores by the compiler. Not intended to
 * synchronize the processor's caches. */
inline void fence() { asm volatile("" : : : "memory"); }

/* 
 * lpf: expected is loaded into EXA, %2 = desired, %1 = obj
 *      if *obj==expected, then *obj = desired, re = expected
 *      if *obj!=expected, then EXA = *obj, re = EXA
 *    Expected in the function is modified, but won't change in the origin function
 */
inline uint64_t cmpxchg(uint64_t *object, uint64_t expected,
                               uint64_t desired) {
  asm volatile("lock; cmpxchgq %2,%1"
               : "+a"(expected), "+m"(*object)
               : "r"(desired)
               : "cc");
  fence();
  return expected;
}

inline uint8_t cmpxchgb(uint8_t *object, uint8_t expected,
                               uint8_t desired) {
  asm volatile("lock; cmpxchgb %2,%1"
               : "+a"(expected), "+m"(*object)
               : "r"(desired)
               : "cc");
  fence();
  return expected;
}

// ========================= seach-schemes ====================

#define SHUF(i0, i1, i2, i3) (i0 + i1*4 + i2*16 + i3*64)
#define FORCEINLINE __attribute__((always_inline)) inline

// power of 2 at most x, undefined for x == 0
FORCEINLINE uint32_t bsr(uint32_t x) {
  return 31 - __builtin_clz(x);
}

static int binary_search_std (const int *arr, int n, int key) {
    return (int) (std::lower_bound(arr, arr + n, key) - arr);
}

static int binary_search_simple(const int *arr, int n, int key) {
  intptr_t left = -1;
  intptr_t right = n;
  while (right - left > 1) {
    intptr_t middle = (left + right) >> 1;
    if (arr[middle] < key)
      left = middle;
    else
      right = middle;
  }
  return (int) right;
}

template<typename KEY_TYPE>
static int binary_search_branchless(const KEY_TYPE *arr, int n, KEY_TYPE key) {
//static int binary_search_branchless(const int *arr, int n, int key) {
  intptr_t pos = -1;
  intptr_t logstep = bsr(n - 1);
  intptr_t step = intptr_t(1) << logstep;

  pos = (arr[pos + n - step] < key ? pos + n - step : pos);
  step >>= 1;

  while (step > 0) {
    pos = (arr[pos + step] < key ? pos + step : pos);
    step >>= 1;
  }
  pos += 1;

  return (int) (arr[pos] >= key ? pos : n);
}

/*static int binary_search_branchless(const int64_t *arr, int n, int64_t key) {
  intptr_t pos = -1;
  intptr_t logstep = bsr(n - 1);
  intptr_t step = intptr_t(1) << logstep;

  pos = (arr[pos + n - step] < key ? pos + n - step : pos);
  step >>= 1;

  while (step > 0) {
    pos = (arr[pos + step] < key ? pos + step : pos);
    step >>= 1;
  }
  pos += 1;

  return (int) (arr[pos] >= key ? pos : n);
}*/

static uint32_t interpolation_search( const int32_t* data, uint32_t n, int32_t key ) {
  uint32_t low = 0;
  uint32_t high = n-1;
  uint32_t mid;

  if ( key <= data[low] ) return low;

  uint32_t iters = 0;
  while ( data[high] > data[low] and
          data[high] > key and
          data[low] < key ) {
    iters += 1;
    if ( iters > 1 ) return binary_search_branchless( data + low, high-low, key );
    
    mid = low + (((long)key - (long)data[low]) * (double)(high - low) / ((long)data[high] - (long)data[low]));

    if ( data[mid] < key ) {
      low = mid + 1;
    } else {
      high = mid - 1;
    }
  }

  if ( key <= data[low] ) return low;
  if ( key <= data[high] ) return high;
  return high + 1;
}

static int linear_search(const int *arr, int n, int key) {
  intptr_t i = 0;
  while (i < n) {
    if (arr[i] >= key)
      break;
    ++i;
  }
  return i;
}

static int linear_search_avx (const int *arr, int n, int key) {
  __m256i vkey = _mm256_set1_epi32(key);
  __m256i cnt = _mm256_setzero_si256();
  for (int i = 0; i < n; i += 16) {
    __m256i mask0 = _mm256_cmpgt_epi32(vkey, _mm256_loadu_si256((__m256i *)&arr[i+0]));
    __m256i mask1 = _mm256_cmpgt_epi32(vkey, _mm256_loadu_si256((__m256i *)&arr[i+8]));
    __m256i sum = _mm256_add_epi32(mask0, mask1);
    cnt = _mm256_sub_epi32(cnt, sum);
  }
  __m128i xcnt = _mm_add_epi32(_mm256_extracti128_si256(cnt, 1), _mm256_castsi256_si128(cnt));
  xcnt = _mm_add_epi32(xcnt, _mm_shuffle_epi32(xcnt, SHUF(2, 3, 0, 1)));
  xcnt = _mm_add_epi32(xcnt, _mm_shuffle_epi32(xcnt, SHUF(1, 0, 3, 2)));
  return _mm_cvtsi128_si32(xcnt);
}

static int linear_search_avx_8(const int *arr, int n, int key) {
    __m256i vkey = _mm256_set1_epi32(key);
    __m256i cnt = _mm256_setzero_si256();

    for (int i = 0; i < n; i += 8) {
        __m256i mask0 = _mm256_cmpgt_epi32(vkey, _mm256_loadu_si256((__m256i *)&arr[i+0]));
        cnt = _mm256_sub_epi32(cnt, mask0);
    }
    //print_256(cnt);

    __m128i xcnt = _mm_add_epi32(_mm256_extracti128_si256(cnt, 1), _mm256_castsi256_si128(cnt));
    xcnt = _mm_add_epi32(xcnt, _mm_shuffle_epi32(xcnt, SHUF(2, 3, 0, 1)));
    xcnt = _mm_add_epi32(xcnt, _mm_shuffle_epi32(xcnt, SHUF(1, 0, 3, 2)));

    //print_128(xcnt);
    //printf("%d\n", _mm_cvtsi128_si32(xcnt));

    return _mm_cvtsi128_si32(xcnt);
}

static void print_256(__m256i key){
    int32_t *p = (int*)&key;
    for (int i = 0; i < 8; i++){
        printf("%d  ", p[i]);
    }
    printf("\n");
}
static int linear_search_avx(const int64_t *arr, int n, int64_t key) {
    __m256i vkey = _mm256_set1_epi64x(key);
    __m256i cnt = _mm256_setzero_si256();
    for (int i = 0; i < n; i += 8) {
      __m256i mask0 = _mm256_cmpgt_epi64(vkey, _mm256_loadu_si256((__m256i *)&arr[i+0]));
      __m256i mask1 = _mm256_cmpgt_epi64(vkey, _mm256_loadu_si256((__m256i *)&arr[i+4]));
      __m256i sum = _mm256_add_epi64(mask0, mask1);
      cnt = _mm256_sub_epi64(cnt, sum);
    }
    __m128i xcnt = _mm_add_epi64(_mm256_extracti128_si256(cnt, 1), _mm256_castsi256_si128(cnt));
    xcnt = _mm_add_epi64(xcnt, _mm_shuffle_epi32(xcnt, SHUF(2, 3, 0, 1)));
    return _mm_cvtsi128_si32(xcnt);
}



// used for masstree and Xindex
#define UNUSED(var) ((void)var)

template <class val_t>
struct AtomicVal {
  union ValUnion;
  typedef ValUnion val_union_t;
  typedef val_t value_type;
  union ValUnion {
    val_t val;
    AtomicVal *ptr;
    ValUnion() {}
    ValUnion(val_t val) : val(val) {}
    ValUnion(AtomicVal *ptr) : ptr(ptr) {}
  };

  // 60 bits for version
  static const uint64_t version_mask = 0x0fffffffffffffff;
  static const uint64_t lock_mask = 0x1000000000000000;
  static const uint64_t removed_mask = 0x2000000000000000;
  static const uint64_t pointer_mask = 0x4000000000000000;

  val_union_t val;
  // lock - removed - is_ptr
  volatile uint64_t status;

  AtomicVal() : status(0) {}
  AtomicVal(val_t val) : val(val), status(0) {}
  AtomicVal(AtomicVal *ptr) : val(ptr), status(0) { set_is_ptr(); }

  bool is_ptr(uint64_t status) { return status & pointer_mask; }
  bool removed(uint64_t status) { return status & removed_mask; }
  bool locked(uint64_t status) { return status & lock_mask; }
  uint64_t get_version(uint64_t status) { return status & version_mask; }

  void set_is_ptr() { status |= pointer_mask; }
  void unset_is_ptr() { status &= ~pointer_mask; }
  void set_removed() { status |= removed_mask; }
  void lock() {
    while (true) {
      uint64_t old = status;
      uint64_t expected = old & ~lock_mask;  // expect to be unlocked
      uint64_t desired = old | lock_mask;    // desire to lock
      /* 
       * lpf: if status == expected, then == expected
       *       else  != expected
       */
      if (likely(cmpxchg((uint64_t *)&this->status, expected, desired) ==
                 expected)) {
        return;
      }
    }
  }
  void unlock() { status &= ~lock_mask; }
  void incr_version() {
    uint64_t version = get_version(status);
    UNUSED(version);
    status++;
    assert(get_version(status) == version + 1);
  }

  friend std::ostream &operator<<(std::ostream &os, const AtomicVal &leaf) {
    COUT_VAR(leaf.val.val);
    COUT_VAR(leaf.val.ptr);
    COUT_VAR(leaf.is_ptr);
    COUT_VAR(leaf.removed);
    COUT_VAR(leaf.locked);
    COUT_VAR(leaf.verion);
    return os;
  }

  // semantics: atomically read the value and the `removed` flag
  bool read(val_t &val) {
    while (true) {
      uint64_t status = this->status;
      memory_fence();
      val_union_t val_union = this->val;
      memory_fence();

      uint64_t current_status = this->status;
      memory_fence();

      if (unlikely(locked(current_status))) {  // check lock
        continue;
      }

      if (likely(get_version(status) ==
                 get_version(current_status))) {  // check version
        if (unlikely(is_ptr(status))) {
          assert(!removed(status));
          return val_union.ptr->read(val);
        } else {
          val = val_union.val;
          return !removed(status);
        }
      }
    }
  }
  bool update(const val_t &val) {
    lock();
    uint64_t status = this->status;
    bool res;
    if (unlikely(is_ptr(status))) {
      assert(!removed(status));
      res = this->val.ptr->update(val);
    } else if (!removed(status)) {
      this->val.val = val;
      res = true;
    } else {
      res = false;
    }
    memory_fence();
    incr_version();
    memory_fence();
    unlock();
    return res;
  }
  bool remove() {
    lock();
    uint64_t status = this->status;
    bool res;
    if (unlikely(is_ptr(status))) {
      assert(!removed(status));
      res = this->val.ptr->remove();
    } else if (!removed(status)) {
      set_removed();
      res = true;
    } else {
      res = false;
    }
    memory_fence();
    incr_version();
    memory_fence();
    unlock();
    return res;
  }
  void replace_pointer() {
    lock();
    uint64_t status = this->status;
    UNUSED(status);
    assert(is_ptr(status));
    assert(!removed(status));
    if (!val.ptr->read(val.val)) {
      set_removed();
    }
    unset_is_ptr();
    memory_fence();
    incr_version();
    memory_fence();
    unlock();
  }
  bool read_ignoring_ptr(val_t &val) {
    while (true) {
      uint64_t status = this->status;
      memory_fence();
      val_union_t val_union = this->val;
      memory_fence();
      if (unlikely(locked(status))) {
        continue;
      }
      memory_fence();

      uint64_t current_status = this->status;
      if (likely(get_version(status) == get_version(current_status))) {
        val = val_union.val;
        return !removed(status);
      }
    }
  }
  bool update_ignoring_ptr(const val_t &val) {
    lock();
    uint64_t status = this->status;
    bool res;
    if (!removed(status)) {
      this->val.val = val;
      res = true;
    } else {
      res = false;
    }
    memory_fence();
    incr_version();
    memory_fence();
    unlock();
    return res;
  }
  bool remove_ignoring_ptr() {
    lock();
    uint64_t status = this->status;
    bool res;
    if (!removed(status)) {
      set_removed();
      res = true;
    } else {
      res = false;
    }
    memory_fence();
    incr_version();
    memory_fence();
    unlock();
    return res;
  }
};



#endif