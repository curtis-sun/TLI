#include <chrono>
#include <cmath>
#include <random>

#include "argparse/argparse.hpp"

#include "rmi/models.hpp"
#include "rmi/rmi.hpp"
#include "rmi/util/fn.hpp"
#include "rmi/util/search.hpp"

using key_type = uint64_t;
using namespace std::chrono;

std::size_t s_glob; ///< global size_t variable


/**
 * Measures lookup times of @p samples on a given @p Rmi and writes results to `std::cout`.
 * @tparam Key key type
 * @tparam Rmi RMI type
 * @tparam Search search type
 * @param keys on which the RMI is built
 * @param n_models number of models in the second layer of the RMI
 * @param samples for which the lookup time is measured
 * @param n_reps number of repetitions
 * @param dataset_name name of the dataset
 * @param layer1 model type of the first layer
 * @param layer2 model type of the second layer
 * @param bounds used by the RMI
 * @param search used by the RMI for correction prediction errors
 * @param budget the budget under which the configuration was chosen
 */
template<typename Key, typename Rmi, typename Search>
void experiment(const std::vector<key_type> &keys,
                const std::size_t n_models,
                const std::vector<key_type> &samples,
                const std::size_t n_reps,
                const std::string dataset_name,
                const std::string layer1,
                const std::string layer2,
                const std::string bounds,
                const std::string search,
                const std::size_t budget,
                const bool is_guideline)
{
    using rmi_type = Rmi;
    auto search_fn = Search();

    // Build RMI.
    rmi_type rmi(keys, n_models);

    // Skip configurations that are guaranteed to not be the fastest.
    if (search == "model_biased_linear") {
        auto n_keys = keys.size();
        std::vector<std::size_t> errors;
        errors.reserve(n_keys);

        for (std::size_t i = 0; i != n_keys; ++i) {
            auto key = keys.at(i);
            auto pred = rmi.search(key).pos;
            auto err = pred > i ? pred - i : i - pred;
            errors.push_back(err);
        }

        auto mean_ae = mean(errors);
        if (mean_ae > 10) return;
    }

    // Perform n_reps runs.
    for (std::size_t rep = 0; rep != n_reps; ++rep) {

        // Lookup time.
        std::size_t lookup_accu = 0;
        auto start = steady_clock::now();
        for (std::size_t i = 0; i != samples.size(); ++i) {
            auto key = samples.at(i);
            auto range = rmi.search(key);
            auto pos = search_fn(keys.begin() + range.lo, keys.begin() + range.hi, keys.begin() + range.pos, key);
            lookup_accu += std::distance(keys.begin(), pos);
        }
        auto stop = steady_clock::now();
        auto lookup_time = duration_cast<nanoseconds>(stop - start).count();
        s_glob = lookup_accu;

        // Report results.
                  // Dataset
        std::cout << dataset_name << ','
                  << keys.size() << ','
                  // Index
                  << layer1 << ','
                  << layer2 << ','
                  << n_models << ','
                  << bounds << ','
                  << search << ','
                  << rmi.size_in_bytes() << ','
                  // Experiment
                  << rep << ','
                  << samples.size() << ','
                  << budget << ','
                  << is_guideline << ','
                  // Results
                  << lookup_time << ','
                  // Checksums
                  << lookup_accu << std::endl;
    } // reps
}


/**
 * @brief experiment function pointer
 */
typedef void (*exp_fn_ptr)(const std::vector<key_type>&,
                           const std::size_t,
                           const std::vector<key_type>&,
                           const std::size_t,
                           const std::string,
                           const std::string,
                           const std::string,
                           const std::string,
                           const std::string,
                           const std::size_t,
                           const bool);


/**
 * RMI configuration that holds the string representation of model types of layer 1 and layer 2, error bound type, and
 * search algorithm.
 */
struct Config {
    std::string layer1;
    std::string layer2;
    std::string bounds;
    std::string search;
};


/**
 * Comparator class for @p Config objects.
 */
struct ConfigCompare {
    bool operator() (const Config &lhs, const Config &rhs) const {
        if (lhs.layer1 != rhs.layer1) return lhs.layer1 < rhs.layer1;
        if (lhs.layer2 != rhs.layer2) return lhs.layer2 < rhs.layer2;
        if (lhs.bounds != rhs.bounds) return lhs.bounds < rhs.bounds;
        return lhs.search < rhs.search;
    }
};


#define ENTRIES(L1, L2, LT1, LT2) \
    { {#L1, #L2, "none", "binary"}, &experiment<key_type, rmi::Rmi<key_type, LT1, LT2>, BinarySearch> }, \
    { {#L1, #L2, "labs", "binary"}, &experiment<key_type, rmi::RmiLAbs<key_type, LT1, LT2>, BinarySearch> }, \
    { {#L1, #L2, "lind", "binary"}, &experiment<key_type, rmi::RmiLInd<key_type, LT1, LT2>, BinarySearch> }, \
    { {#L1, #L2, "gabs", "binary"}, &experiment<key_type, rmi::RmiGAbs<key_type, LT1, LT2>, BinarySearch> }, \
    { {#L1, #L2, "gind", "binary"}, &experiment<key_type, rmi::RmiGInd<key_type, LT1, LT2>, BinarySearch> }, \
    { {#L1, #L2, "none", "model_biased_binary"}, &experiment<key_type, rmi::Rmi<key_type, LT1, LT2>, ModelBiasedBinarySearch> }, \
    { {#L1, #L2, "labs", "model_biased_binary"}, &experiment<key_type, rmi::RmiLAbs<key_type, LT1, LT2>, ModelBiasedBinarySearch> }, \
    { {#L1, #L2, "lind", "model_biased_binary"}, &experiment<key_type, rmi::RmiLInd<key_type, LT1, LT2>, ModelBiasedBinarySearch> }, \
    { {#L1, #L2, "gabs", "model_biased_binary"}, &experiment<key_type, rmi::RmiGAbs<key_type, LT1, LT2>, ModelBiasedBinarySearch> }, \
    { {#L1, #L2, "gind", "model_biased_binary"}, &experiment<key_type, rmi::RmiGInd<key_type, LT1, LT2>, ModelBiasedBinarySearch> }, \
    { {#L1, #L2, "none", "linear"}, &experiment<key_type, rmi::Rmi<key_type, LT1, LT2>, LinearSearch> }, \
    { {#L1, #L2, "labs", "linear"}, &experiment<key_type, rmi::RmiLAbs<key_type, LT1, LT2>, LinearSearch> }, \
    { {#L1, #L2, "lind", "linear"}, &experiment<key_type, rmi::RmiLInd<key_type, LT1, LT2>, LinearSearch> }, \
    { {#L1, #L2, "gabs", "linear"}, &experiment<key_type, rmi::RmiGAbs<key_type, LT1, LT2>, LinearSearch> }, \
    { {#L1, #L2, "gind", "linear"}, &experiment<key_type, rmi::RmiGInd<key_type, LT1, LT2>, LinearSearch> }, \
    { {#L1, #L2, "none", "model_biased_linear"}, &experiment<key_type, rmi::Rmi<key_type, LT1, LT2>, ModelBiasedLinearSearch> }, \
    { {#L1, #L2, "labs", "model_biased_linear"}, &experiment<key_type, rmi::RmiLAbs<key_type, LT1, LT2>, ModelBiasedLinearSearch> }, \
    { {#L1, #L2, "lind", "model_biased_linear"}, &experiment<key_type, rmi::RmiLInd<key_type, LT1, LT2>, ModelBiasedLinearSearch> }, \
    { {#L1, #L2, "gabs", "model_biased_linear"}, &experiment<key_type, rmi::RmiGAbs<key_type, LT1, LT2>, ModelBiasedLinearSearch> }, \
    { {#L1, #L2, "gind", "model_biased_linear"}, &experiment<key_type, rmi::RmiGInd<key_type, LT1, LT2>, ModelBiasedLinearSearch> }, \
    { {#L1, #L2, "none", "exponential"}, &experiment<key_type, rmi::Rmi<key_type, LT1, LT2>, ExponentialSearch> }, \
    { {#L1, #L2, "labs", "exponential"}, &experiment<key_type, rmi::RmiLAbs<key_type, LT1, LT2>, ExponentialSearch> }, \
    { {#L1, #L2, "lind", "exponential"}, &experiment<key_type, rmi::RmiLInd<key_type, LT1, LT2>, ExponentialSearch> }, \
    { {#L1, #L2, "gabs", "exponential"}, &experiment<key_type, rmi::RmiGAbs<key_type, LT1, LT2>, ExponentialSearch> }, \
    { {#L1, #L2, "gind", "exponential"}, &experiment<key_type, rmi::RmiGInd<key_type, LT1, LT2>, ExponentialSearch> }, \
    { {#L1, #L2, "none", "model_biased_exponential"}, &experiment<key_type, rmi::Rmi<key_type, LT1, LT2>, ModelBiasedExponentialSearch> }, \
    { {#L1, #L2, "labs", "model_biased_exponential"}, &experiment<key_type, rmi::RmiLAbs<key_type, LT1, LT2>, ModelBiasedExponentialSearch> }, \
    { {#L1, #L2, "lind", "model_biased_exponential"}, &experiment<key_type, rmi::RmiLInd<key_type, LT1, LT2>, ModelBiasedExponentialSearch> }, \
    { {#L1, #L2, "gabs", "model_biased_exponential"}, &experiment<key_type, rmi::RmiGAbs<key_type, LT1, LT2>, ModelBiasedExponentialSearch> }, \
    { {#L1, #L2, "gind", "model_biased_exponential"}, &experiment<key_type, rmi::RmiGInd<key_type, LT1, LT2>, ModelBiasedExponentialSearch> }, \

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


/*
 * Computes the recommended RMI configuration by following a simple guideline and evaluates its performance.
 * @param keys on which the RMI is built
 * @param samples for which the lookup time is measured
 * @param n_reps number of repetitions
 * @param dataset_name name of the dataset
 * @param budget the budget under which the configuration is to be chosen
 */
void evaluate_guideline(const std::vector<key_type> &keys,
                        const std::vector<key_type> &samples,
                        const std::size_t n_reps,
                        const std::string dataset_name,
                        const std::size_t budget)
{
    // Dermine maximum number of layer 2 models for LS->LR NB+MExp.
    auto n_models = (budget - 2 * sizeof(double) - 2 * sizeof(std::size_t)) / (2 * sizeof(double));

    // Train RMI.
    rmi::Rmi<key_type, rmi::LinearSpline, rmi::LinearRegression> rmi(keys, n_models);

    // Evaluate RMI error.
    auto n_keys = keys.size();
    std::vector<double> log2_errors;
    log2_errors.reserve(n_keys);

    for (std::size_t i = 0; i != n_keys; ++i) {
        auto key = keys.at(i);
        auto pred = rmi.search(key).pos;
        auto err = pred > i ? pred - i : i - pred;
        log2_errors.push_back(std::log2(err+1));
    }

    auto mean_log2e = mean(log2_errors);

    // Pick and evaluate guideline config based on errors.
    auto l1 = "linear_spline";
    auto l2 = "linear_regression";

    auto threshold = 5.8; // This is hardware-dependent.

    if (mean_log2e < threshold) {
        auto bounds = "none";
        auto search = "model_biased_exponential";

        Config config {l1, l2, bounds, search};
        exp_fn_ptr exp_fn = exp_map[config];

        (*exp_fn)(keys, n_models, samples, n_reps, dataset_name, l1, l2, bounds, search, budget, true);
    } else {
        auto bounds = "labs";
        auto search = "binary";
        n_models = (budget - 2 * sizeof(double) - 2 * sizeof(std::size_t)) / (2 * sizeof(double) + sizeof(std::size_t));

        Config config {l1, l2, bounds, search};
        exp_fn_ptr exp_fn = exp_map[config];

        (*exp_fn)(keys, n_models, samples, n_reps, dataset_name, l1, l2, bounds, search, budget, true);
    }
}


/**
 * Tests RMI configurations for a given size budget and compares them against the performance chosen by the guideline in
 * termns of lookup time.
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

    program.add_argument("budget")
        .help("target size in bytes for the RMI configurations to test")
        .action([](const std::string &s) { return std::stoul(s); });

   program.add_argument("-n", "--n_reps")
        .help("number of experiment repetitions")
        .default_value(std::size_t(3))
        .action([](const std::string &s) { return std::stoul(s); });

    program.add_argument("-s", "--n_samples")
        .help("number of sampled lookup keys")
        .default_value(std::size_t(1'000'000))
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
    const auto budget = program.get<std::size_t>("budget");
    const auto n_reps = program.get<std::size_t>("-n");
    const auto n_samples = program.get<std::size_t>("-s");

    // Load keys.
    auto keys = load_data<key_type>(filename);

    // Sample keys.
    uint64_t seed = 42;
    std::mt19937 gen(seed);
    std::uniform_int_distribution<> distrib(0, keys.size() - 1);
    std::vector<key_type> samples;
    samples.reserve(n_samples);
    for (std::size_t i = 0; i != n_samples; ++i)
        samples.push_back(keys[distrib(gen)]);

    // List configuration parameters.
    std::vector<std::string> l1_models = {"linear_spline", "cubic_spline", "linear_regression", "radix"};
    std::vector<std::string> l2_models = {"linear_regression"}; // We know that lr is always better than ls from previous experiments.
    std::vector<std::pair<std::string, std::string>> err_corrs = {
        std::make_pair("none", "model_biased_exponential"),
        std::make_pair("none", "model_biased_linear"),
        std::make_pair("labs", "binary"),
        std::make_pair("lind", "model_biased_binary"),
    };

    // List model and bound sizes.
    std::map<std::string, std::size_t> model_size = {
        { "linear_spline", 2 * sizeof(double) },
        { "cubic_spline", 4 * sizeof(double) },
        { "linear_regression", 2 * sizeof(double) },
        { "radix", 1 * sizeof(key_type) },
    };
    std::map<std::string, std::size_t> bounds_size = {
        { "none", 0 },
        { "labs", sizeof(std::size_t) },
        { "lind", 2 * sizeof(std::size_t) },
    };

    // Output header.
    if (program["--header"]  == true)
        std::cout << "dataset,"
                  << "n_keys,"
                  << "layer1,"
                  << "layer2,"
                  << "n_models,"
                  << "bounds,"
                  << "search,"
                  << "size_in_bytes,"
                  << "rep,"
                  << "n_samples,"
                  << "budget_in_bytes,"
                  << "is_guideline,"
                  << "lookup_time,"
                  << "lookup_accu"
                  << std::endl;

    // Enumerate and evaluate configurations.
    for (auto l1 : l1_models) {
        for (auto l2 : l2_models) {
            for (auto corr : err_corrs) {
                auto bounds = corr.first;
                auto search = corr.second;

                // Dermine maximum number of layer 2 models.
                auto n_models = (budget - model_size[l1] - 2 * sizeof(std::size_t)) / (model_size[l2] + bounds_size[bounds]);

                // Build configuiration object.
                Config config {l1, l2, bounds, search};

                // Lookup evaluation function.
                exp_fn_ptr exp_fn = exp_map[config];

                // Call evaluatin function with keys and n_models.
                (*exp_fn)(keys, n_models, samples, n_reps, dataset_name, l1, l2, bounds, search, budget, false);
            }
        }
    }

    // Evaluate guideline configuration.
    evaluate_guideline(keys, samples, n_reps, dataset_name, budget);

    exit(EXIT_SUCCESS);
}
