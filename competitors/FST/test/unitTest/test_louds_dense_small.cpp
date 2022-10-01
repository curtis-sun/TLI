#include "gtest/gtest.h"

#include <assert.h>

#include <string>
#include <vector>

#include "config.hpp"
#include "fst.hpp"

namespace surf {

namespace surftest {

static const bool kIncludeDense = true;
static const uint32_t kSparseDenseRatio = 0;
static const SuffixType kSuffixType = kReal;
static const level_t kSuffixLen = 8;

class SuRFInt32Test : public ::testing::Test {
 public:
  virtual void SetUp() {}

  virtual void TearDown() {}
};

TEST_F (SuRFInt32Test, ExampleInPaperTest) {
  std::vector<std::string> keys;
  std::vector<uint64_t> values;

  keys.emplace_back(std::string("f"));
  keys.emplace_back(std::string("far"));
  keys.emplace_back(std::string("fas"));
  keys.emplace_back(std::string("fast"));
  keys.emplace_back(std::string("fat"));
  keys.emplace_back(std::string("s"));
  keys.emplace_back(std::string("top"));
  keys.emplace_back(std::string("toy"));
  keys.emplace_back(std::string("trie"));
  keys.emplace_back(std::string("trip"));
  keys.emplace_back(std::string("try"));
  for (auto i = 0; i < keys.size(); i++) {
    values.emplace_back(i);
  }
  auto *builder = new SuRFBuilder(kIncludeDense, kSparseDenseRatio);
  builder->build(keys, values);
  auto *louds_dense = new LoudsDense(builder);
  LoudsDense::Iter iter(louds_dense);

  louds_dense->moveToKeyGreaterThan(std::string("to"), true, iter);
  ASSERT_TRUE(iter.isValid());
  ASSERT_EQ(0, iter.getKey().compare("top"));
  iter++;
  ASSERT_EQ(0, iter.getKey().compare("toy"));

  iter.clear();
  louds_dense->moveToKeyGreaterThan(std::string("fas"), true, iter);
  ASSERT_TRUE(iter.isValid());

}

} // namespace surftest

} // namespace surf

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
