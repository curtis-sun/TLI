#include "argparse/argparse.hpp"

#include "rmi/models.hpp"
#include "rmi/util/fn.hpp"

using key_type = uint64_t;


/**
 * Computes statistical properties of the segments created when segmenting the @p keys with @p Model and writes results
 * to `std::cout`.
 * @tparam Key key type
 * @tparam Model model type
 * @param keys on which the RMI is built
 * @param n_segments number of segments to be created
 * @param dataset_name name of the dataset
 * @param model model type used for segementing the keys
 */
template<typename Key, typename Model>
void experiment(const std::vector<key_type> &keys,
                const std::size_t n_segments,
                const std::string dataset_name,
                const std::string model)
{
    using model_type = Model;

    // Build model.
    auto m = model_type(keys.begin(), keys.end(), 0, static_cast<double>(n_segments) / keys.size());

    // Initialize variables.
    std::vector<std::size_t> segments(n_segments, 0);

    // Segment keys.
    for (std::size_t i =0; i != keys.size(); ++i) {
        auto key = keys.at(i);
        std::size_t segment = std::clamp<double>(m.predict(key), 0, n_segments - 1);
        segments[segment]++;
    }

    // Compute properties.
    auto n_empty = std::count(segments.begin(), segments.end(), 0);

    // Report results.
                 // Dataset
    std::cout << dataset_name << ','
              << keys.size() << ','
                 // Model config
              << model << ','
              << n_segments << ','
                 // Absolute error
              << mean(segments) << ','
              << stdev(segments) << ','
              << median(segments) << ','
              << min(segments) << ','
              << max(segments) << ','
              << n_empty << std::endl;
}

/**
 * @brief experiment function pointer
 */
typedef void (*exp_fn_ptr)(const std::vector<key_type>&,
                           const std::size_t,
                           const std::string,
                           const std::string);

#define ENTRY(L, T) \
    { #L, &experiment<key_type, T> }

static std::map<std::string, exp_fn_ptr> exp_map {
    ENTRY(linear_regression, rmi::LinearRegression),
    ENTRY(linear_spline,     rmi::LinearSpline),
    ENTRY(cubic_spline,      rmi::CubicSpline),
    ENTRY(radix,             rmi::Radix<key_type>),
}; ///< Map that assigns an experiment function pointer to model types.
#undef ENTRY


/**
 * Performs segmentation using a model type and segment count provided via command line arguemnt and reports several
 * statistical properties of the resulting segments.
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

    program.add_argument("model")
        .help("model type, either linear_regression, linear_spline, cubic_spline, or radix.");

    program.add_argument("n_segments")
        .help("number of segments, power of two is recommended.")
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
    const auto model = program.get<std::string>("model");
    const auto n_segments = program.get<std::size_t>("n_segments");

    // Load keys.
    auto keys = load_data<key_type>(filename);

    // Lookup experiment.
    if (exp_map.find(model) == exp_map.end()) {
        std::cerr << "Error: " << model << " is not a valid model type." << std::endl;
        exit(EXIT_FAILURE);
    }
    exp_fn_ptr exp_fn = exp_map[model];

    // Output header.
    if (program["--header"]  == true)
        std::cout << "dataset,"
                  << "n_keys,"
                  << "model,"
                  << "n_segments,"
                  << "mean,"
                  << "stdev,"
                  << "median,"
                  << "min,"
                  << "max,"
                  << "n_empty"
                  << std::endl;

    // Run experiment.
    (*exp_fn)(keys, n_segments, dataset_name, model);

    exit(EXIT_SUCCESS);
}
