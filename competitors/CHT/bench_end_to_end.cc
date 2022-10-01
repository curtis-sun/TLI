#include <chrono>
#include <fstream>
#include <iostream>
#include <map>

#include "include/cht/builder.h"
#include "include/cht/cht.h"

using namespace std;

namespace util {

// Loads values from binary file into vector.
template <typename T>
static vector<T> load_data(const string& filename, bool print = true) {
  vector<T> data;
  ifstream in(filename, ios::binary);
  if (!in.is_open()) {
    cerr << "unable to open " << filename << endl;
    exit(EXIT_FAILURE);
  }
  // Read size.
  uint64_t size;
  in.read(reinterpret_cast<char*>(&size), sizeof(uint64_t));
  data.resize(size);
  // Read values.
  in.read(reinterpret_cast<char*>(data.data()), size * sizeof(T));

  return data;
}

// Generates deterministic values for keys.
template <class KeyType>
static vector<pair<KeyType, uint64_t>> add_values(const vector<KeyType>& keys) {
  vector<pair<KeyType, uint64_t>> result;
  result.reserve(keys.size());

  for (uint64_t i = 0; i < keys.size(); ++i) {
    pair<KeyType, uint64_t> row;
    row.first = keys[i];
    row.second = i;

    result.push_back(row);
  }
  return result;
}

}  // namespace util

namespace {

template <class KeyType, class ValueType>
class NonOwningMultiMap {
 public:
  using element_type = pair<KeyType, ValueType>;

  NonOwningMultiMap(const vector<element_type>& elements,
                    const uint32_t num_bins, const uint32_t max_error,
                    const bool single_pass, const bool ccht)
      : data_(elements) {
    assert(elements.size() > 0);

    // Create builder.
    const auto min_key = data_.front().first;
    const auto max_key = data_.back().first;
    cht::Builder<KeyType> chtb(min_key, max_key, num_bins, max_error,
                               single_pass, ccht);

    // Build the index.
    for (const auto& iter : data_) {
      chtb.AddKey(iter.first);
    }
    cht_ = chtb.Finalize();
  }

  typename vector<element_type>::const_iterator lower_bound(KeyType key) const {
    cht::SearchBound bound = cht_.GetSearchBound(key);
    return ::lower_bound(data_.begin() + bound.begin, data_.begin() + bound.end,
                         key, [](const element_type& lhs, const KeyType& rhs) {
                           return lhs.first < rhs;
                         });
  }

  uint64_t sum_up(KeyType key) const {
    uint64_t result = 0;
    auto iter = lower_bound(key);
    while (iter != data_.end() && iter->first == key) {
      result += iter->second;
      iter++;
    }
    return result;
  }

  size_t GetSizeInByte() const { return cht_.GetSize(); }

 private:
  const vector<element_type>& data_;
  cht::CompactHistTree<KeyType> cht_;
};

template <class KeyType>
struct Lookup {
  KeyType key;
  uint64_t value;
};

template <class KeyType>
void Run(const string& data_file, const string lookup_file,
         const uint32_t num_bins, const uint32_t max_error,
         const bool single_pass, const bool ccht) {
  // Load data
  std::cerr << "Load data.." << std::endl;
  vector<KeyType> keys = util::load_data<KeyType>(data_file);
  vector<pair<KeyType, uint64_t>> elements = util::add_values(keys);
  vector<Lookup<KeyType>> lookups =
      util::load_data<Lookup<KeyType>>(lookup_file);

  // Build index
  std::cerr << "Build index.." << std::endl;
  auto build_begin = chrono::high_resolution_clock::now();
  NonOwningMultiMap<KeyType, uint64_t> map(elements, num_bins, max_error,
                                           single_pass, ccht);
  auto build_end = chrono::high_resolution_clock::now();
  uint64_t build_ns =
      chrono::duration_cast<chrono::nanoseconds>(build_end - build_begin)
          .count();

  // Run queries
  std::cerr << "Run queries.." << std::endl;
  vector<uint64_t> lookup_ns;
  for (uint32_t i = 0; i < 3; i++) {
    auto lookup_begin = chrono::high_resolution_clock::now();
    for (const Lookup<KeyType>& lookup_iter : lookups) {
      uint64_t sum = map.sum_up(lookup_iter.key);
      if (sum != lookup_iter.value) {
        cerr << "wrong result!" << endl;
        throw "error";
      }
    }
    auto lookup_end = chrono::high_resolution_clock::now();
    uint64_t run_lookup_ns =
        chrono::duration_cast<chrono::nanoseconds>(lookup_end - lookup_begin)
            .count();
    lookup_ns.push_back(run_lookup_ns / lookups.size());
  }
  sort(lookup_ns.begin(), lookup_ns.end());

  cout << data_file << "," << num_bins << "," << max_error << "," << single_pass
       << "," << ccht << ","
       << static_cast<double>(map.GetSizeInByte()) / 1000 / 1000 << ","
       << static_cast<double>(build_ns) / 1000 / 1000 / 1000 << ","
       << lookup_ns[1] << endl;
}

}  // namespace

int main(int argc, char** argv) {
  if (argc != 7) {
    cerr << "usage: " << argv[0]
         << " <data_file> <lookup_file> <num_bins> <max_error> <single_pass> "
            "<ccht>"
         << endl;
    throw;
  }
  const string data_file = argv[1];
  const string lookup_file = argv[2];
  const uint32_t num_bins = atoi(argv[3]);
  const uint32_t max_error = atoi(argv[4]);
  const bool single_pass = atoi(argv[5]);
  const bool ccht = atoi(argv[6]);

  if (data_file.find("32") != string::npos) {
    Run<uint32_t>(data_file, lookup_file, num_bins, max_error, single_pass,
                  ccht);
  } else {
    Run<uint64_t>(data_file, lookup_file, num_bins, max_error, single_pass,
                  ccht);
  }

  return 0;
}