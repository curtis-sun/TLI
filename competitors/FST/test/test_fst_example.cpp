#include "gtest/gtest.h"
#include <string>
#include <vector>
#include "config.hpp"
#include "fst.hpp"

namespace fst {

namespace surftest {

class SuRFExampleWords : public ::testing::Test {
 public:
  void SetUp() override {
    keys.emplace_back(std::string("abca"));
    keys.emplace_back(std::string("abcb"));
    keys.emplace_back(std::string("ac"));
    keys.emplace_back(std::string("adef"));
    keys.emplace_back(std::string("adeg"));
    keys.emplace_back(std::string("aef"));
    keys.emplace_back(std::string("aeg"));
    keys.emplace_back(std::string("b"));

    values_uint64.resize(keys.size());

    // generate keys and values
    for (size_t i = 0; i < values_uint64.size(); i++) {
      values_uint64[i] = i;
    }
  }

  void TearDown() override {}

  std::vector<std::string> keys;
  std::vector<uint64_t> values_uint64;
};

TEST_F (SuRFExampleWords, IteratorTest) {
  FST *surf = new FST(keys, values_uint64, kIncludeDense, 100);
  auto iterators = surf->lookupRange("a", true, "b", false);
  uint64_t i(0);
  if (iterators.first.isValid())
    i = iterators.first.getValue();
  while (iterators.first != iterators.second) {
    iterators.first.getValue();
    std::cout << iterators.first.getKey() << ",\t\t" << iterators.first.getValue() << std::endl;
    ASSERT_EQ(iterators.first.getValue(), i);
    iterators.first++;
    i++;
  }
  //if (iterators.second.isValid())
  //    std::cout << iterators.first.getKey() << ",\t\t" << iterators.first.getValue() << std::endl;
}
} // namespace surftest


} // namespace fst

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
