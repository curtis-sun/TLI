#include "include/cht/cht.h"

#include <random>
#include <unordered_set>

#include "gtest/gtest.h"
#include "include/cht/builder.h"

const size_t kNumKeys = 1000;
// Number of iterations (seeds) of random positive and negative test cases.
const size_t kNumIterations = 10;
const size_t kNumBins = 32;
const size_t kMaxError = 32;

namespace {

// *** Helper methods ***

template <class KeyType>
std::vector<KeyType> CreateDenseKeys() {
  std::vector<KeyType> keys;
  keys.reserve(kNumKeys);
  for (size_t i = 0; i < kNumKeys; ++i) keys.push_back(i);
  return keys;
}

template <class KeyType>
std::vector<KeyType> CreateUniqueRandomKeys(size_t seed) {
  std::unordered_set<KeyType> keys;
  keys.reserve(kNumKeys);
  std::mt19937 g(seed);
  std::uniform_int_distribution<KeyType> d(std::numeric_limits<KeyType>::min(),
                                           std::numeric_limits<KeyType>::max());
  while (keys.size() < kNumKeys) keys.insert(d(g));
  std::vector<KeyType> sorted_keys(keys.begin(), keys.end());
  std::sort(sorted_keys.begin(), sorted_keys.end());
  return sorted_keys;
}

template <class KeyType>
cht::CompactHistTree<KeyType> CreateCompactHistTree(const std::vector<KeyType>& keys) {
  auto min = std::numeric_limits<KeyType>::min();
  auto max = std::numeric_limits<KeyType>::max();
  if (keys.size() > 0) {
    min = keys.front();
    max = keys.back();
  }
  cht::Builder<KeyType> chtb(min, max, kNumBins, kMaxError);
  for (const auto& key : keys) chtb.AddKey(key);
  return chtb.Finalize();
}

template <class KeyType>
bool BoundContains(const std::vector<KeyType>& keys, cht::SearchBound bound,
                   KeyType key) {
  const auto it = std::lower_bound(keys.begin() + bound.begin,
                                   keys.begin() + bound.end, key);
  if (it == keys.end()) return false;
  return *it == key;
}

// *** Tests ***

template <class T>
struct CompactHistTreeTest : public testing::Test {
  using KeyType = T;
};

using AllKeyTypes = testing::Types<uint32_t, uint64_t>;
TYPED_TEST_SUITE(CompactHistTreeTest, AllKeyTypes);

TYPED_TEST(CompactHistTreeTest, AddAndLookupDenseKeys) {
  using KeyType = typename TestFixture::KeyType;
  const auto keys = CreateDenseKeys<KeyType>();
  const auto cht = CreateCompactHistTree(keys);
  for (const auto& key : keys)
    EXPECT_TRUE(BoundContains(keys, cht.GetSearchBound(key), key))
        << "key: " << key;
}


TYPED_TEST(CompactHistTreeTest, AddAndLookupRandomKeysPositiveLookups) {
  using KeyType = typename TestFixture::KeyType;
  for (size_t i = 0; i < kNumIterations; ++i) {
    const auto keys = CreateUniqueRandomKeys<KeyType>(/*seed=*/i);
    const auto cht = CreateCompactHistTree(keys);
    for (const auto& key : keys)
      EXPECT_TRUE(BoundContains(keys, cht.GetSearchBound(key), key))
          << "key: " << key;
  }
}

TYPED_TEST(CompactHistTreeTest, AddAndLookupRandomIntegechtNegativeLookups) {
  using KeyType = typename TestFixture::KeyType;
  for (size_t i = 0; i < kNumIterations; ++i) {
    const auto keys = CreateUniqueRandomKeys<KeyType>(/*seed=*/42 + i);
    const auto lookup_keys = CreateUniqueRandomKeys<KeyType>(/*seed=*/815 + i);
    const auto cht = CreateCompactHistTree(keys);
    for (const auto& key : lookup_keys) {
      if (!BoundContains(keys, cht::SearchBound{0, keys.size()}, key))
        EXPECT_FALSE(BoundContains(keys, cht.GetSearchBound(key), key))
            << "key: " << key;
    }
  }
}

}  // namespace
