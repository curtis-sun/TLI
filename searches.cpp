# include "./util.h"
#include "utils/cxxopts.hpp"
# include "searches/branching_binary_search.h"
# include "searches/exponential_search.h"
# include "searches/interpolation_search.h"
# include "searches/linear_search.h"
# include "searches/linear_search_avx.h"
# include <iostream>
# include <random>
using namespace std;

template<class KeyType, class SearchClass>
double computeResult(const vector<KeyType>& keys, size_t start_pos, size_t num, size_t error, mt19937_64& generator){
    const size_t sample_size = std::min(size_t(100000), num);
    uniform_int_distribution<int> coin(0, 1);
    vector<pair<size_t, size_t>> search_points;
    search_points.reserve(sample_size);
    for (unsigned i = 0; i < sample_size; i ++){
        uniform_int_distribution<KeyType> dis(error, num - 1 - error);
        size_t target = dis(generator);
        size_t predict = error ? (coin(generator) ? target + error : target - error) : target;
        // size_t accurate;
        search_points.push_back({target, predict});
        // if (keys[accurate] != keys[start_pos + target]){
        //     std::cerr << "Search returned wrong result:" << std::endl;
        //     std::cerr << "Lookup key: " << keys[start_pos + target] << " at " << start_pos + target << std::endl;
        //     std::cerr << "Actual key: " << keys[accurate] << " at " << accurate << std::endl;
        // }
    }
    uint64_t time = util::timing([&](){
        for (unsigned i = 0; i < sample_size; i ++){
            SearchClass::lower_bound(keys.begin() + start_pos, keys.begin() + start_pos + num, keys[start_pos + search_points[i].first], keys.begin() + start_pos + search_points[i].second) - keys.begin();
        }
    });
    return double(time) / sample_size;
}

template<class KeyType>
void printResult(const vector<KeyType>& keys, size_t start_pos, size_t num, size_t error, mt19937_64& generator, const string& filename){
    ofstream fout(filename, std::ofstream::out | std::ofstream::app);

    if (!fout.is_open()) {
      cerr << "Failure to print CSV on " << filename << std::endl;
      return;
    }

    fout << num << ',' << error << ',' << "linear," << 
            computeResult<KeyType, LinearSearch<0>>(keys, start_pos, num, error, generator) << endl;
    fout << num << ',' << error << ',' << "binary," << 
            computeResult<KeyType, BranchingBinarySearch<0>>(keys, start_pos, num, error, generator) << endl;
    fout << num << ',' << error << ',' << "avx," << 
            computeResult<KeyType, LinearAVX<KeyType, 0>>(keys, start_pos, num, error, generator) << endl;
    fout << num << ',' << error << ',' << "exp," << 
            computeResult<KeyType, ExponentialSearch<0>>(keys, start_pos, num, error, generator) << endl;
    fout << num << ',' << error << ',' << "interp," << 
            computeResult<KeyType, InterpolationSearch<false>>(keys, start_pos, num, error, generator) << endl;
}

template<class KeyType>
void test(const string& filename, size_t start_pos, size_t end_pos, size_t error_bound) {
    static constexpr const char* prefix = "data/";
    string dataset_name = filename.data();
    dataset_name.erase(
    dataset_name.begin(),
    dataset_name.begin() + dataset_name.find(prefix) + strlen(prefix));
    const string result_name = "./results/" + dataset_name + "_" 
                + to_string(start_pos) + "st_"
                + to_string(end_pos) + "ed_"
                + to_string(error_bound) + "bd_results_table.csv";
    mt19937_64 generator(42);
    // Load data.
    vector<KeyType> keys = util::load_data<KeyType>(filename);
    if (!is_sorted(keys.begin(), keys.end()))
        util::fail("keys have to be sorted");

    end_pos = std::min(end_pos, keys.size());
    if (start_pos >= end_pos){
        util::fail("start position must be smaller than end position");
    }

    size_t num = 1;
    for (size_t i = 0; i <= floor(log2(end_pos - start_pos)); i ++) {
        size_t error = 0;
        printResult(keys, start_pos, num, error, generator, result_name);
        error = 1;
        for (size_t j = 0; j + 1 < i && error <= error_bound; j ++) {
            printResult(keys, start_pos, num, error, generator, result_name);
            error <<= 1;
        }
        num <<= 1;
    }
        
}

int main(int argc, char* argv[]){
    cxxopts::Options options("searches", "Evaluate search methods on sorted data");
    options.positional_help("<data>");
    options.add_options()("data", "Data file with keys",
                        cxxopts::value<string>())(
      "help", "Displays help")("s,start-pos", "Start position", 
                               cxxopts::value<size_t>()->default_value("0"))(
      "e,end-pos", "End position", 
                               cxxopts::value<size_t>()->default_value(to_string(numeric_limits<size_t>::max())))(
      "err-bound", "Error bound", 
                               cxxopts::value<size_t>()->default_value(to_string(numeric_limits<size_t>::max())));

    options.parse_positional({"data"});

    const auto result = options.parse(argc, argv);

    if (result.count("help")) {
        cout << options.help({}) << "\n";
        exit(0);
    }

    const string filename = result["data"].as<string>();
    const DataType type = util::resolve_type(filename);
    size_t start_pos = result["start-pos"].as<size_t>();
    size_t end_pos = result["end-pos"].as<size_t>();
    size_t error_bound = result["err-bound"].as<size_t>();

    switch (type) {
        case DataType::UINT32: {
            test<uint32_t>(filename, start_pos, end_pos, error_bound);
            break;
        }

        case DataType::UINT64: {
            test<uint64_t>(filename, start_pos, end_pos, error_bound);
            break;
        }
    }
    return 0;
}