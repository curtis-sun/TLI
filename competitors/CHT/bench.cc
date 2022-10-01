#include <chrono>
#include <fstream>
#include <iostream>
#include <random>

#include "include/cht/builder.h"
#include "include/cht/cht.h"

using namespace std::chrono;

template <class KeyType>
auto RandomNumberBetween = [](KeyType low, KeyType high) {
  auto randomFunc =
      [distribution_ = std::uniform_int_distribution<KeyType>(low, high),
       random_engine_ = std::mt19937{std::random_device{}()}]() mutable {
        return distribution_(random_engine_);
      };
  return randomFunc;
};

template <class KeyType>
static std::vector<KeyType> Generate(unsigned size, KeyType maxValue,
                                     bool withSort = false) {
  std::vector<KeyType> numbers;
  std::generate_n(std::back_inserter(numbers), size / 4,
                  RandomNumberBetween<KeyType>(0, maxValue / 4));
  std::generate_n(std::back_inserter(numbers), size / 2,
                  RandomNumberBetween<KeyType>(maxValue / 4, maxValue / 2));
  std::generate_n(std::back_inserter(numbers), size / 4,
                  RandomNumberBetween<KeyType>(maxValue / 2, maxValue));
  if (withSort) sort(std::begin(numbers), std::end(numbers));
  return numbers;
}

template <class KeyType>
void CreateInput(std::vector<KeyType>& keys, std::vector<KeyType>& queries) {
  keys = Generate(1e6, std::numeric_limits<KeyType>::max(), true);
  queries = Generate(1e6, std::numeric_limits<KeyType>::max());
  std::cout << "Generated " << keys.size() << " keys and " << queries.size()
            << " lookup keys!" << std::endl;
}

template <class KeyType>
void Compare() {
  std::vector<KeyType> keys, queries;
  CreateInput<KeyType>(keys, queries);

#if 1
  std::ofstream out("compare_keys.out");
  for (unsigned index = 0; index != keys.size(); ++index)
    out << keys[index] << std::endl;
  out << std::endl;
#endif

  for (unsigned i = 1; i <= 10; ++i) {
    for (unsigned j = 1; j <= 10; ++j) {
      auto numBins = 1u << i, maxError = 1u << j;
      std::cerr << "numBins=" << numBins << " maxError=" << maxError
                << std::endl;

      // Build both CHTs
      KeyType min = keys.front();
      KeyType max = keys.back();

      cht::Builder<KeyType> spchtb(min, max, numBins, maxError, true, false);
      cht::Builder<KeyType> ochtb(min, max, numBins, maxError, false, false);
      for (const auto& key : keys) spchtb.AddKey(key), ochtb.AddKey(key);
      auto spcht = spchtb.Finalize();
      auto ocht = ochtb.Finalize();

      // Compare pure lookups
      auto Compare = [&]() -> void {
        for (auto query : queries) {
          auto b1 = spcht.GetSearchBound(query);
          auto b2 = ocht.GetSearchBound(query);
          assert(b1.first == b2.first && b1.second == b2.second);
        }
      };

      Compare();
    }
  }
}

template <class KeyType>
void Benchmark(bool single_pass) {
  std::vector<KeyType> keys, queries;
  CreateInput<KeyType>(keys, queries);

#if 1
  std::ofstream out("benchmark_keys.out");
  for (unsigned index = 0; index != keys.size(); ++index)
    out << keys[index] << std::endl;
  out << std::endl;
#endif

  for (unsigned i = 1; i <= 10; ++i) {
    for (unsigned j = 1; j <= 10; ++j) {
      auto numBins = 1u << i, maxError = 1u << j;

      // Build both CHTs
      KeyType min = keys.front();
      KeyType max = keys.back();

      cht::Builder<KeyType> chtb(min, max, numBins, maxError, single_pass,
                                 false);
      cht::Builder<KeyType> cchtb(min, max, numBins, maxError, single_pass,
                                  true);
      for (const auto& key : keys) chtb.AddKey(key), cchtb.AddKey(key);
      auto cht = chtb.Finalize();
      auto ccht = cchtb.Finalize();

      // Compare pure lookups
      auto measureTime = [&](std::string type) -> void {
        auto start = high_resolution_clock::now();
        if (type == "CHT") {
          for (auto query : queries) {
            cht.GetSearchBound(query);
          }
        } else if (type == "CCHT") {
          for (auto query : queries) {
            ccht.GetSearchBound(query);
          }
        }

        auto stop = high_resolution_clock::now();
        double answer = duration_cast<nanoseconds>(stop - start).count();
        std::cout << type << "<"
                  << (std::is_same<KeyType, uint32_t>::value ? "uint32_t"
                                                             : "uint64_t")
                  << ">(numBins=" << numBins << ", maxError=" << maxError
                  << ", single_pass=" << single_pass << "): " << answer << " ns"
                  << std::endl;
      };

      for (unsigned index = 0; index != 5; ++index) {
        measureTime("CHT");
        measureTime("CCHT");
      }
    }
  }
}

int main(void) {
#if 0
	Compare<uint32_t>();
	Compare<uint64_t>();
#endif

  // Benchmark (needs big RAM)
  Benchmark<uint32_t>(true);
  Benchmark<uint64_t>(true);
  Benchmark<uint32_t>(false);
  Benchmark<uint64_t>(false);
  return 0;
}
