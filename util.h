#pragma once

#include <algorithm>
#include <cassert>
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <thread>
#include <unordered_map>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <cstring>
#include <immintrin.h>
#include <atomic>

//#define PRINT_ERRORS

#if !defined(forceinline)
#ifdef _MSC_VER
#define forceinline __forceinline
#elif defined(__GNUC__)
#define forceinline inline __attribute__((__always_inline__))
#elif defined(__CLANG__)
#if __has_attribute(__always_inline__)
#define forceinline inline __attribute__((__always_inline__))
#else
#define forceinline inline
#endif
#else
#define forceinline inline
#endif
#endif

enum DataType { UINT32 = 0, UINT64 = 1, STRING = 2 };

// Dataset key-value pair.
template <class KeyType>
struct KeyValue {
  KeyType key;
  uint64_t value;
} __attribute__((packed));

template <class KeyType>
struct alignas(2) Element {
  KeyType key;
  uint64_t value;
  Element(const KeyType& k, uint64_t v): key(k), value(v){}
};

// Workload operation.
template <class KeyType = uint64_t>
struct Operation {
  uint8_t op;
  KeyType lo_key;
  KeyType hi_key;
  uint64_t result;
} __attribute__((packed));

#define CACHELINE_SIZE (1 << 6)
// Thread information.
struct alignas(CACHELINE_SIZE) FGParam{
  void *index, *ops, *keys;
  uint64_t *individual_ns;
  uint64_t start, limit, op_cnt;
  uint32_t thread_id;
};

__m256i static forceinline _mm256_cmpge_epu32(__m256i a, __m256i b) {
  return _mm256_cmpeq_epi32(_mm256_max_epu32(a, b), a);
}

__m256i static forceinline _mm256_cmple_epu32(__m256i a, __m256i b) {
  return _mm256_cmpge_epu32(b, a);
}

__m256i static forceinline _mm256_cmpgt_epu32(__m256i a, __m256i b) {
  return _mm256_xor_si256(_mm256_cmple_epu32(a, b), _mm256_set1_epi32(-1));
}

__m256i static forceinline _mm256_cmplt_epu32(__m256i a, __m256i b) {
  return _mm256_cmpgt_epu32(b, a);
}

__m256i static forceinline _mm256_cmpgt_epu64(__m256i a, __m256i b) {  
  const static __m256i highBit = _mm256_set1_epi64x((long long)0x8000000000000000);   
  a = _mm256_xor_si256(a, highBit);
  b = _mm256_xor_si256(b, highBit);
  return _mm256_cmpgt_epi64(a, b);
}

__m256i static forceinline _mm256_cmplt_epu64(__m256i a, __m256i b) {  
  return _mm256_cmpgt_epu64(b, a);
}

__m256i static forceinline _mm256_cmpge_epu64(__m256i a, __m256i b) {  
  return _mm256_xor_si256(_mm256_cmplt_epu64(a, b), _mm256_set1_epi32(-1));
}

__m256i static forceinline _mm256_cmple_epu64(__m256i a, __m256i b) {  
  return _mm256_cmpge_epu64(b, a);
}

namespace util {

const static uint8_t LOOKUP = 0;
const static uint8_t RANGE_QUERY = 1;
const static uint8_t INSERT = 2;

const static uint64_t NOT_FOUND = std::numeric_limits<uint64_t>::max();

const static size_t OVERFLOW = std::numeric_limits<size_t>::max();

static volatile bool running = false;
static std::atomic<size_t> ready_threads;

static void fail(const std::string& message) {
  std::cerr << message << std::endl;
  exit(EXIT_FAILURE);
}

template <class KeyType>
[[maybe_unused]] static void convert2String(const KeyType& key, std::string& str){
  KeyType endian_key;
  if constexpr (std::is_same<KeyType, uint32_t>::value){
    endian_key = __builtin_bswap32(key);
  }
  else if constexpr (std::is_same<KeyType, uint64_t>::value){
    endian_key = __builtin_bswap64(key);
  }
  else{
    util::fail("Undefined key type.");
  }
  str = std::string(reinterpret_cast<const char*>(&endian_key), sizeof(KeyType));
}

template <>
[[maybe_unused]] void convert2String(const std::string& key, std::string& str){
  str = key;
  str.push_back(0);
}

[[maybe_unused]] static std::string get_suffix(const std::string& filename) {
  const std::size_t pos = filename.find_last_of("_");
  if (pos == filename.size() - 1 || pos == std::string::npos) return "";
  return filename.substr(pos + 1);
}

[[maybe_unused]] static DataType resolve_type(const std::string& filename) {
  const std::string suffix = util::get_suffix(filename);
  if (suffix == "uint32") {
    return DataType::UINT32;
  } else if (suffix == "uint64") {
    return DataType::UINT64;
  } else if (suffix == "string") {
    return DataType::STRING;
  } else {
    std::cerr << "type " << suffix << " not supported" << std::endl;
    exit(EXIT_FAILURE);
  }
}

// Pins the current thread to core `core_id`.
static void set_cpu_affinity(const uint32_t core_id) __attribute__((unused));
static void set_cpu_affinity(const uint32_t core_id) {
#ifdef __linux__
  cpu_set_t mask;
  CPU_ZERO(&mask);
  CPU_SET(core_id % std::thread::hardware_concurrency(), &mask);
  const int result =
      pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask);
  if (result != 0) fail("failed to set CPU affinity");
#else
  (void)core_id;
  std::cout << "we only support thread pinning under Linux" << std::endl;
#endif
}

static uint64_t timing(std::function<void()> fn) {
  const auto start = std::chrono::high_resolution_clock::now();
  fn();
  const auto end = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start)
      .count();
}

// Checks whether data is duplicate free.
// Note that data has to be sorted.
template <typename T>
static bool is_unique(const std::vector<T>& data) {
  for (size_t i = 1; i < data.size(); ++i) {
    if (data[i] == data[i - 1]) return false;
  }
  return true;
}

template <class KeyType>
static bool is_unique(const std::vector<KeyValue<KeyType>>& data) {
  for (size_t i = 1; i < data.size(); ++i) {
    if (data[i].key == data[i - 1].key) return false;
  }
  return true;
}

// Load from binary file into vector.
template <typename T>
static std::vector<T> in_data(std::ifstream& in){
  // Read size.
  uint64_t size;
  in.read(reinterpret_cast<char*>(&size), sizeof(uint64_t));
  std::vector<T> data(size);
  // Read data.
  if constexpr (std::is_same<T, std::string>::value){
    for (size_t i = 0; i < size; i ++){
      uint32_t len;
      in.read(reinterpret_cast<char*>(&len), sizeof(uint32_t));
      char str[len];
      in.read(reinterpret_cast<char*>(str), len);
      data[i] = std::string(str, len);
    }
  }
  else if constexpr (std::is_same<T, Operation<std::string>>::value){
    for (size_t i = 0; i < size; i ++){
      in.read(reinterpret_cast<char*>(&data[i].op), sizeof(uint8_t));
      in.read(reinterpret_cast<char*>(&data[i].result), sizeof(uint64_t));
      uint32_t len;
      in.read(reinterpret_cast<char*>(&len), sizeof(uint32_t));
      char lo_str[len];
      in.read(reinterpret_cast<char*>(lo_str), len);
      data[i].lo_key = std::string(lo_str, len);
      in.read(reinterpret_cast<char*>(&len), sizeof(uint32_t));
      char hi_str[len];
      in.read(reinterpret_cast<char*>(hi_str), len);
      data[i].hi_key = std::string(hi_str, len);
    }
  }
  else if constexpr (std::is_same<T, KeyValue<std::string>>::value){
    for (size_t i = 0; i < size; i ++){
      in.read(reinterpret_cast<char*>(&data[i].value), sizeof(uint64_t));
      uint32_t len;
      in.read(reinterpret_cast<char*>(&len), sizeof(uint32_t));
      char str[len];
      in.read(reinterpret_cast<char*>(str), len);
      data[i].key = std::string(str, len);
    }
  }
  else{
    in.read(reinterpret_cast<char*>(data.data()), size * sizeof(T));
  }
  return data;
}

template <typename T>
static std::vector<T> load_data(const std::string& filename,
                                bool print = true) {
  std::vector<T> data;
  const uint64_t ns = util::timing([&] {
    std::ifstream in(filename, std::ios::binary);
    if (!in.is_open()) {
      std::cerr << "unable to open " << filename << std::endl;
      exit(EXIT_FAILURE);
    }
    data = in_data<T>(in);
    in.close();
  });
  const uint64_t ms = ns / 1e6;

  if (print) {
    std::cout << "read " << data.size() << " values from " << filename << " in "
              << ms << " ms (" << static_cast<double>(data.size()) / 1000 / ms
              << " M values/s)" << std::endl;
  }

  return data;
}

template <typename T>
static std::vector<std::vector<T>> load_data_multithread(const std::string& filename,
                                bool print = true) {
  std::vector<std::vector<T>> data;
  size_t size = 0;
  const uint64_t ns = util::timing([&] {
    std::ifstream in(filename, std::ios::binary);
    if (!in.is_open()) {
      std::cerr << "unable to open " << filename << std::endl;
      exit(EXIT_FAILURE);
    }
    // Read thread number.
    uint64_t len;
    in.read(reinterpret_cast<char*>(&len), sizeof(uint64_t));
    data.reserve(len);
    for (size_t i = 0; i < len; i ++){
      data.push_back(in_data<T>(in));
      size += data[i].size();
    }
    in.close();
  });
  const uint64_t ms = ns / 1e6;

  if (print) {
    std::cout << "read " << size << " values from " << filename << " in "
              << ms << " ms (" << static_cast<double>(size) / 1000 / ms
              << " M values/s)" << std::endl;
  }

  return data;
}

// Write from vector into binary file.
template <typename T>
static void out_data(const std::vector<T>& data, std::ofstream& out) {
    // Write size.
    const uint64_t size = data.size();
    out.write(reinterpret_cast<const char*>(&size), sizeof(uint64_t));
    // Write data.
    if constexpr (std::is_same<T, std::string>::value){
      for (size_t i = 0; i < size; i ++){
        uint32_t len = data[i].length();
        out.write(reinterpret_cast<const char*>(&len), sizeof(uint32_t));
        out.write(reinterpret_cast<const char*>(data[i].c_str()), len);
      }
    }
    else if constexpr (std::is_same<T, Operation<std::string>>::value){
      for (size_t i = 0; i < size; i ++){
        out.write(reinterpret_cast<const char*>(&data[i].op), sizeof(uint8_t));
        out.write(reinterpret_cast<const char*>(&data[i].result), sizeof(uint64_t));
        uint32_t len = data[i].lo_key.length();
        out.write(reinterpret_cast<const char*>(&len), sizeof(uint32_t));
        out.write(reinterpret_cast<const char*>(data[i].lo_key.c_str()), len);
        len = data[i].hi_key.length();
        out.write(reinterpret_cast<const char*>(&len), sizeof(uint32_t));
        out.write(reinterpret_cast<const char*>(data[i].hi_key.c_str()), len);
      }
    }
    else if constexpr (std::is_same<T, KeyValue<std::string>>::value){
      for (size_t i = 0; i < size; i ++){
        out.write(reinterpret_cast<const char*>(&data[i].value), sizeof(uint64_t));
        uint32_t len = data[i].key.length();
        out.write(reinterpret_cast<const char*>(&len), sizeof(uint32_t));
        out.write(reinterpret_cast<const char*>(data[i].key.c_str()), len);
      }
    }
    else{
      out.write(reinterpret_cast<const char*>(data.data()), size * sizeof(T));
    }
}

template <typename T>
static void write_data(const std::vector<T>& data, const std::string& filename,
                       const bool print = true) {
  const uint64_t ns = util::timing([&] {
    std::ofstream out(filename, std::ios_base::trunc | std::ios::binary);
    if (!out.is_open()) {
      std::cerr << "unable to open " << filename << std::endl;
      exit(EXIT_FAILURE);
    }
    out_data(data, out);
    out.close();
  });
  const uint64_t ms = ns / 1e6;
  if (print) {
    std::cout << "wrote " << data.size() << " values to " << filename << " in "
              << ms << " ms (" << static_cast<double>(data.size()) / 1000 / ms
              << " M values/s)" << std::endl;
  }
}

// Write from vector into binary file.
template <typename T>
static void write_data_multithread(std::vector<T> const* data, uint64_t len, const std::string& filename,
                       const bool print = true) {
  size_t size = 0;
  const uint64_t ns = util::timing([&] {
    std::ofstream out(filename, std::ios_base::trunc | std::ios::binary);
    if (!out.is_open()) {
      std::cerr << "unable to open " << filename << std::endl;
      exit(EXIT_FAILURE);
    }
    // Write thread number.
    out.write(reinterpret_cast<const char*>(&len), sizeof(uint64_t));
    for (size_t i = 0; i < len; ++ i){
      out_data(data[i], out);
      size += data[i].size();
    }
    out.close();
  });
  const uint64_t ms = ns / 1e6;
  if (print) {
    std::cout << "wrote " << size << " values to " << filename << " in "
              << ms << " ms (" << static_cast<double>(size) / 1000 / ms
              << " M values/s)" << std::endl;
  }
}

// Generate deterministic values for keys.
template <class KeyType>
static std::vector<KeyValue<KeyType>> add_values(const std::vector<KeyType>& keys) {
  std::vector<KeyValue<KeyType>> result;
  result.reserve(keys.size());

  for (uint64_t i = 0; i < keys.size(); ++i) {
    KeyValue<KeyType> kv;
    kv.key = keys[i];
    kv.value = i;
    result.push_back(kv);
  }
  return result;
}

// Based on: https://en.wikipedia.org/wiki/Xorshift
class FastRandom {
 public:
  explicit FastRandom(
      uint64_t seed = 2305843008139952128ull)  // The 8th perfect number found
                                               // 1772 by Euler with <3
      : seed(seed) {}
  uint32_t RandUint32() {
    seed ^= (seed << 13);
    seed ^= (seed >> 15);
    return (uint32_t)(seed ^= (seed << 5));
  }
  int32_t RandInt32() { return (int32_t)RandUint32(); }
  uint32_t RandUint32(uint32_t inclusive_min, uint32_t inclusive_max) {
    return inclusive_min + RandUint32() % (inclusive_max - inclusive_min + 1);
  }
  int32_t RandInt32(int32_t inclusive_min, int32_t inclusive_max) {
    return inclusive_min + RandUint32() % (inclusive_max - inclusive_min + 1);
  }
  float RandFloat(float inclusive_min, float inclusive_max) {
    return inclusive_min + ScaleFactor() * (inclusive_max - inclusive_min);
  }
  // returns float between 0 and 1
  float ScaleFactor() {
    return static_cast<float>(RandUint32()) /
           std::numeric_limits<uint32_t>::max();
  }
  bool RandBool() { return RandUint32() % 2 == 0; }

  uint64_t seed;

  static constexpr uint64_t Min() { return 0; }
  static constexpr uint64_t Max() {
    return std::numeric_limits<uint64_t>::max();
  }
};

}  // namespace util
