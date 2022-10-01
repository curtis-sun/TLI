#pragma once

#include <cstddef>
#include <cstdint>

namespace ts {

// A CDF coordinate.
template <class KeyType>
struct Coord {
  KeyType x;
  double y;
};

struct SearchBound {
  size_t begin;
  size_t end;  // Exclusive.
};

// A radix config.
struct RadixConfig {
	unsigned shiftBits;
	unsigned prevPrefix;
	unsigned prevSplineIndex;
	double cost;
};

// Statistics.
struct Statistics {
  Statistics() {}
  
  Statistics(unsigned numBins, unsigned treeMaxError, double cost, size_t space)
    : numBins(numBins), treeMaxError(treeMaxError),
      cost(cost), space(space) {}

  unsigned numBins;
  unsigned treeMaxError;
  double cost;
  size_t space;
};

}  // namespace ts
