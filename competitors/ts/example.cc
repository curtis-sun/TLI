#include <algorithm>
#include <iostream>
#include <vector>

#include "include/ts/builder.h"
#include "include/ts/common.h"

using namespace std;

void TrieSplineExample() {
  // Create random keys.
  std::vector<uint64_t> keys(1e6);
  generate(keys.begin(), keys.end(), rand);
  keys.push_back(424242);
  std::sort(keys.begin(), keys.end());

  // Build TS
  uint64_t min = keys.front();
  uint64_t max = keys.back();
  ts::Builder<uint64_t> tsb(min, max, /*spline_max_error=*/32);

  for (const auto& key : keys) tsb.AddKey(key);
  auto ts = tsb.Finalize();

  // Search using TS
  ts::SearchBound bound = ts.GetSearchBound(424242);
  std::cout << "The search key is in the range: [" << bound.begin << ", "
            << bound.end << ")" << std::endl;
  auto start = std::begin(keys) + bound.begin,
       last = std::begin(keys) + bound.end;
  auto pos = std::lower_bound(start, last, 424242) - begin(keys);
  assert(keys[pos] == 424242);
  std::cout << "The key is at position: " << pos << std::endl;
}

int main(int argc, char** argv) {
  TrieSplineExample();
  return 0;
}
