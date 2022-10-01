#include "gtest/gtest.h"
#include <string>
#include <vector>
#include "config.hpp"
#include "fst.hpp"
#include <chrono>
#include <fstream>

namespace fst {

namespace surftest {

static const std::string kFilePath = "keys.txt";
static const int kTestSize = 4000;

class SuRFExampleWords : public ::testing::Test {
 public:
  void SetUp() override {
    keys.resize(kTestSize);
    values_uint64.resize(kTestSize);
    loadWordList();
  }

  void loadWordList() {
    auto start = std::chrono::high_resolution_clock::now();
    std::ifstream infile(kFilePath);
    std::string key;
    int count = 0;
    while (infile.good() && count < kTestSize) {
      infile >> key;
      keys[count] = key;
      values_uint64[count] = count;
      count++;
    }
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    std::cout << "read keys in " << std::to_string(elapsed.count()) << " seconds" << std::endl;

  }

  void TearDown() override {}

  std::vector<std::string> keys;
  std::vector<uint64_t> values_uint64;
};

TEST_F (SuRFExampleWords, MapTest) {
  std::vector<std::pair<std::string, uint64_t >> initializer_list(keys.size());

  for (auto i = 0u; i < keys.size(); i++) {
    initializer_list.emplace_back(std::make_pair(keys[i], values_uint64[i]));
  }

  auto start = std::chrono::high_resolution_clock::now();
  std::map<std::string, uint64_t> map(initializer_list.begin(), initializer_list.end());
  auto finish = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = finish - start;
  std::cout << "build time " << std::to_string(elapsed.count()) << std::endl;

  uint64_t total_size(0);
  for (auto const &element : map) {
    total_size += element.first.size() + 8;

  }
  std::cout << "Size of map<string,uint64_t>: " << (total_size / (1024 * 1024)) << " MiB" << std::endl;
}

TEST_F (SuRFExampleWords, IteratorTest1) {
  // build fst
  auto start = std::chrono::high_resolution_clock::now();
  FST *surf = new FST(keys, values_uint64, kIncludeDense, 16);
  auto finish = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = finish - start;
  std::cout << "build time " << std::to_string(elapsed.count()) << std::endl;

  // query fst range
  start = std::chrono::high_resolution_clock::now();
  auto iterators = surf->lookupRange(keys.front(), true, keys.back(), true);
  uint64_t i(0);

  while (iterators.first != iterators.second) {
    ASSERT_EQ(iterators.first.getValue(), i);
    iterators.first++;
    i++;
  }

  finish = std::chrono::high_resolution_clock::now();
  elapsed = finish - start;

  std::cout << "range lookup TP [M/s]: " << std::to_string(kTestSize * 1.00 / (elapsed.count() * 1000000))
            << std::endl;


  // query fst key lookup
  start = std::chrono::high_resolution_clock::now();

  i = 0;
  uint64_t value = -1;
  while (i < keys.size()) {
    surf->lookupKey(keys[i], value);
    ASSERT_EQ(value, values_uint64[i]);
    i++;
  }

  finish = std::chrono::high_resolution_clock::now();
  elapsed = finish - start;

  std::cout << "point lookup TP [M/s]: " << std::to_string(kTestSize * 1.00 / (elapsed.count() * 1000000))
            << std::endl;

  size_t surf_mib = surf->getMemoryUsage() / (1024 * 1024);
  std::cout << surf_mib << " MiB" << std::endl;
}
} // namespace surftest

} // namespace fst

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
