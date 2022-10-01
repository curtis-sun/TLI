"Hist-Tree" - Andrew Crotty, Brown University
====

Implementation of the compact version of the Hist-Tree. A cache-oblivious version is also considered.

## Build

```
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
./example
```

## Examples

Using ``cht::Builder`` to index the sorted data:

```c++
// Create random keys.
std::vector<uint64_t> keys(1e6);
generate(keys.begin(), keys.end(), rand);
keys.push_back(424242);
std::sort(keys.begin(), keys.end());

// Build `CompactHistTree`
uint64_t min = keys.front();
uint64_t max = keys.back();
const unsigned numBins = 64; // each node will have 64 separate bins
const unsigned maxError = 32; // the error of the index
cht::Builder<uint64_t> chtb(min, max, numBins, maxError);
for (const auto& key : keys) chtb.AddKey(key);
cht::CompactHistTree<uint64_t> cht = chtb.Finalize();

// Search using `CompactHistTree`
cht::SearchBound bound = cht.GetSearchBound(424242);
std::cout << "The search key is in the range: ["
			<< bound.begin << ", " << bound.end << ")" << std::endl;
auto start = std::begin(keys) + bound.begin, last = std::begin(keys) + bound.end;
auto pos = std::lower_bound(start, last, 424242) - begin(keys);
assert(keys[pos] == 424242);
std::cout << "The key is at position: " << pos << std::endl;
```
