#include "argparse/argparse.hpp"

#include "rmi/models.hpp"
#include "rmi/rmi.hpp"
#include "rmi/util/fn.hpp"

using key_type = uint64_t;


/**
 * Computes several error metrics for a given @p Rmi on dataset @p keys and writes results to `std::cout`.
 * @tparam Key key type
 * @tparam Rmi RMI type
 * @param keys on which the RMI is built
 * @param n_models number of models in the second layer of the RMI
 * @param dataset_name name of the dataset
 * @param layer1 model type of the first layer
 * @param layer2 model type of the second layer
 */
template<typename Key, typename Rmi>
void experiment(const std::vector<key_type> &keys,
                const std::size_t n_models,
                const std::string dataset_name,
                const std::string layer1,
                const std::string layer2)
{
    using rmi_type = Rmi;

    // Build RMI.
    rmi_type rmi(keys, n_models);

    // Initialize variables.
    auto n_keys = keys.size();
    std::vector<int64_t> absolute_errors;
    absolute_errors.reserve(n_keys);

    // Perform predictions.
    auto prev_key = keys.at(0);
    int64_t prev_pos = 0;
    for (std::size_t i = 0; i != n_keys; ++i) {
        auto key = keys.at(i);
        auto pred = rmi.search(key);

        // Record error.
        int64_t pos = key == prev_key ? prev_pos : i;
        auto absolute_error = std::abs(pos - static_cast<int64_t>(pred.pos));
        absolute_errors.push_back(absolute_error);

        prev_key = key;
        prev_pos = pos;
    }

    // Report results.
                 // Dataset
    std::cout << dataset_name << ','
              << n_keys << ','
                 // RMI config
              << layer1 << ','
              << layer2 << ','
              << n_models << ','
                 // Absolute error
              << mean(absolute_errors) << ','
              << median(absolute_errors) << ','
              << stdev(absolute_errors) << ','
              << min(absolute_errors) << ','
              << max(absolute_errors) << std::endl;
}


/**
 * @brief experiment function pointer
 */
typedef void (*exp_fn_ptr)(const std::vector<key_type>&,
                           const std::size_t,
                           const std::string,
                           const std::string,
                           const std::string);

#define ENTRY(L1, L2, T1, T2) \
    { std::make_pair(#L1, #L2), &experiment<key_type, rmi::Rmi<key_type, T1, T2>> }

static std::map<std::pair<std::string, std::string>, exp_fn_ptr> exp_map {
    ENTRY(linear_regression, linear_regression, rmi::LinearRegression, rmi::LinearRegression),
    ENTRY(linear_regression, linear_spline,     rmi::LinearRegression, rmi::LinearSpline),
    ENTRY(linear_spline,     linear_regression, rmi::LinearSpline,     rmi::LinearRegression),
    ENTRY(linear_spline,     linear_spline,     rmi::LinearSpline,     rmi::LinearSpline),
    ENTRY(cubic_spline,      linear_regression, rmi::CubicSpline,      rmi::LinearRegression),
    ENTRY(cubic_spline,      linear_spline,     rmi::CubicSpline,      rmi::LinearSpline),
    ENTRY(radix,             linear_regression, rmi::Radix<key_type>,  rmi::LinearRegression),
    ENTRY(radix,             linear_spline,     rmi::Radix<key_type>,  rmi::LinearSpline),
}; ///< Map that assigns an experiment function pointer to RMI configurations.
#undef ENTRY


/**
 * Triggers computation of several error metrics of an RMI configuration provided via command line arguments.
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

    // Load keys.
    auto keys = load_data<key_type>(filename);

    // Lookup experiment.
    auto config = std::make_pair(layer1, layer2);
    if (exp_map.find(config) == exp_map.end()) {
        std::cerr << "Error: " << layer1 << ',' << layer2 << " is not a valid RMI configuration." << std::endl;
        exit(EXIT_FAILURE);
    }
    exp_fn_ptr exp_fn = exp_map[config];

    // Output header.
    if (program["--header"]  == true)
        std::cout << "dataset,"
                  << "n_keys,"
                  << "layer1,"
                  << "layer2,"
                  << "n_models,"
                  << "mean_ae,"
                  << "median_ae,"
                  << "stdev_ae"
                  << "min_ae"
                  << "max_ae"
                  << std::endl;

    // Run experiment.
    (*exp_fn)(keys, n_models, dataset_name, layer1, layer2);

    exit(EXIT_SUCCESS);
}
