#include <algorithm>
#include <chrono>

#include "argparse/argparse.hpp"
#include "rmi/models.hpp"
#include "rmi/rmi.hpp"

using key_type = uint64_t;
using namespace std::chrono;

std::size_t s_glob; ///< global size_t variable


/**
 * Measures the build time for a given @p Rmi on dataset @p keys and writes results to `std::cout`.
 * @tparam Key key type
 * @tparam Rmi RMI type
 * @param keys on which the RMI is built
 * @param n_models number of models in the second layer of the RMI
 * @param dataset_name name of the dataset
 * @param layer1 model type of the first layer
 * @param layer2 model type of the second layer
 * @param bounds_type used by the RMI
 */
template<typename Key, typename Rmi>
void experiment(const std::vector<key_type> &keys,
                const std::size_t n_models,
                const std::size_t n_reps,
                const std::string dataset_name,
                const std::string layer1,
                const std::string layer2,
                const std::string bound_type)
{
    using rmi_type = Rmi;

    // Perform n_reps runs.
    for (std::size_t rep = 0; rep != n_reps; ++rep) {

        // Build RMI.
        auto start = steady_clock::now();
        rmi_type rmi(keys, n_models);
        auto stop = steady_clock::now();
        auto build_time = duration_cast<nanoseconds>(stop - start).count();

        // Perform lookup to ensure that RMI is actually built.
        auto key = keys.at(0);
        auto range = rmi.search(key);
        auto pos = std::lower_bound(keys.begin() + range.lo, keys.begin() + range.hi, key);
        s_glob = std::distance(keys.begin(), pos);

        // Report results.
                  // Dataset
        std::cout << dataset_name << ','
                  << keys.size() << ','
                  // Index
                  << "ours" << ','
                  << layer1 << ','
                  << layer2 << ','
                  << n_models << ','
                  << bound_type << ','
                  << rmi.size_in_bytes() << ','
                  // Experiment
                  << rep << ','
                  // Results
                  << build_time << ','
                  // Checksums
                  << s_glob << std::endl;
    } // reps
}


/**
 * @brief experiment function pointer
 */
typedef void (*exp_fn_ptr)(const std::vector<key_type>&,
                           const std::size_t,
                           const std::size_t,
                           const std::string,
                           const std::string,
                           const std::string,
                           const std::string);

/**
 * RMI configuration that holds the string representation of model types of layer 1 and layer 2 and the error bound
 * type.
 */
struct Config {
    std::string layer1;
    std::string layer2;
    std::string bound_type;
};

/**
 * Comparator class for @p Config objects.
 */
struct ConfigCompare {
    bool operator() (const Config &lhs, const Config &rhs) const {
        if (lhs.layer1 != rhs.layer1) return lhs.layer1 < rhs.layer1;
        if (lhs.layer2 != rhs.layer2) return lhs.layer2 < rhs.layer2;
        return lhs.bound_type < rhs.bound_type;
    }
};

#define ENTRIES(L1, L2, T1, T2) \
    { {#L1, #L2, "labs"}, &experiment<key_type, rmi::RmiLAbs<key_type, T1, T2>> }, \
    { {#L1, #L2, "lind"}, &experiment<key_type, rmi::RmiLInd<key_type, T1, T2>> }, \
    { {#L1, #L2, "gabs"}, &experiment<key_type, rmi::RmiGAbs<key_type, T1, T2>> }, \
    { {#L1, #L2, "gind"}, &experiment<key_type, rmi::RmiGInd<key_type, T1, T2>> }, \
    { {#L1, #L2, "none"}, &experiment<key_type, rmi::Rmi<key_type, T1, T2>> },

static std::map<Config, exp_fn_ptr, ConfigCompare> exp_map {
    ENTRIES(linear_regression, linear_regression, rmi::LinearRegression, rmi::LinearRegression)
    ENTRIES(linear_regression, linear_spline,     rmi::LinearRegression, rmi::LinearSpline)
    ENTRIES(linear_spline,     linear_regression, rmi::LinearSpline,     rmi::LinearRegression)
    ENTRIES(linear_spline,     linear_spline,     rmi::LinearSpline,     rmi::LinearSpline)
    ENTRIES(cubic_spline,      linear_regression, rmi::CubicSpline,      rmi::LinearRegression)
    ENTRIES(cubic_spline,      linear_spline,     rmi::CubicSpline,      rmi::LinearSpline)
    ENTRIES(radix,             linear_regression, rmi::Radix<key_type>,  rmi::LinearRegression)
    ENTRIES(radix,             linear_spline,     rmi::Radix<key_type>,  rmi::LinearSpline)
}; ///< Map that assigns an experiment function pointer to RMI configurations.
#undef ENTRIES


/**
 * Triggers measurement of build times for an RMI configuration provided via command line arguments.
 * @param argc arguments counter
 * @param argv arguments vector
 */
int main(int argc, char *argv[])
{
    // Initialize argument parser.
    argparse::ArgumentParser program(argv[0], "0.1");

    // Define arguments.
    program.add_argument("filename")
        .help("path to binary file containing uin64_t keys");

    program.add_argument("layer1")
        .help("layer1 model type, either linear_regression, linear_spline, cubic_spline, or radix.");

    program.add_argument("layer2")
        .help("layer2 model type, either linear_regression, linear_spline, or cubic_spline.");

    program.add_argument("n_models")
        .help("number of models on layer2, power of two is recommended.")
        .action([](const std::string &s) { return std::stoul(s); });

    program.add_argument("bound_type")
        .help("type of error bounds used, either none, labs, lind, gabs, or gind.");

   program.add_argument("-n", "--n_reps")
        .help("number of experiment repetitions")
        .default_value(std::size_t(3))
        .action([](const std::string &s) { return std::stoul(s); });

    program.add_argument("--header")
        .help("output csv header")
        .default_value(false)
        .implicit_value(true);

    // Parse arguments.
    try {
        program.parse_args(argc, argv);
    }
    catch (const std::runtime_error &err) {
        std::cout << err.what() << '\n' << program;
        exit(EXIT_FAILURE);
    }

    // Read arguments.
    const auto filename = program.get<std::string>("filename");
    const auto dataset_name = split(filename, '/').back();
    const auto layer1 = program.get<std::string>("layer1");
    const auto layer2 = program.get<std::string>("layer2");
    const auto n_models = program.get<std::size_t>("n_models");
    const auto bound_type = program.get<std::string>("bound_type");
    const auto n_reps = program.get<std::size_t>("-n");

    // Load keys.
    auto keys = load_data<key_type>(filename);

    // Lookup experiment.
    Config config{layer1, layer2, bound_type};
    if (exp_map.find(config) == exp_map.end()) {
        std::cerr << "Error: " << layer1 << ',' << layer2 << ',' << bound_type <<  " is not a valid RMI configuration." << std::endl;
        exit(EXIT_FAILURE);
    }
    exp_fn_ptr exp_fn = exp_map[config];

    // Output header.
    if (program["--header"]  == true)
        std::cout << "dataset,"
                  << "n_keys,"
                  << "rmi,"
                  << "layer1,"
                  << "layer2,"
                  << "n_models,"
                  << "bounds,"
                  << "size_in_bytes,"
                  << "rep,"
                  << "build_time,"
                  << "checksum"
                  << std::endl;

    // Run experiment.
    (*exp_fn)(keys, n_models, n_reps, dataset_name, layer1, layer2, bound_type);

    exit(EXIT_SUCCESS);
}
