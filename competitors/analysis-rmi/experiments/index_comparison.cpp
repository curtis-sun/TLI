#include <iostream>
#include <random>

#include "argparse/argparse.hpp"

#include "rmi/models.hpp"
#include "rmi/rmi.hpp"
#include "rmi/util/fn.hpp"
#include "rmi/util/search.hpp"

#include "core/alex.h"
#include "core/alex_base.h"

#include "pgm/pgm_index.hpp"

#include "rs/builder.h"
#include "rs/radix_spline.h"

#include "cht/builder.h"
#include "cht/cht.h"

#include "art/art.hpp"

#include "tlx/container/btree_multimap.hpp"

#include "rmi_ref/books_200M_uint64_0.h"
#include "rmi_ref/books_200M_uint64_1.h"
#include "rmi_ref/books_200M_uint64_2.h"
#include "rmi_ref/books_200M_uint64_3.h"
#include "rmi_ref/books_200M_uint64_4.h"
#include "rmi_ref/books_200M_uint64_5.h"
#include "rmi_ref/books_200M_uint64_6.h"
#include "rmi_ref/books_200M_uint64_7.h"
#include "rmi_ref/books_200M_uint64_8.h"
#include "rmi_ref/books_200M_uint64_9.h"
#include "rmi_ref/fb_200M_uint64_0.h"
#include "rmi_ref/fb_200M_uint64_1.h"
#include "rmi_ref/fb_200M_uint64_2.h"
#include "rmi_ref/fb_200M_uint64_3.h"
#include "rmi_ref/fb_200M_uint64_4.h"
#include "rmi_ref/fb_200M_uint64_5.h"
#include "rmi_ref/fb_200M_uint64_6.h"
#include "rmi_ref/fb_200M_uint64_7.h"
#include "rmi_ref/fb_200M_uint64_8.h"
#include "rmi_ref/fb_200M_uint64_9.h"
#include "rmi_ref/osm_cellids_200M_uint64_0.h"
#include "rmi_ref/osm_cellids_200M_uint64_1.h"
#include "rmi_ref/osm_cellids_200M_uint64_2.h"
#include "rmi_ref/osm_cellids_200M_uint64_3.h"
#include "rmi_ref/osm_cellids_200M_uint64_4.h"
#include "rmi_ref/osm_cellids_200M_uint64_5.h"
#include "rmi_ref/osm_cellids_200M_uint64_6.h"
#include "rmi_ref/osm_cellids_200M_uint64_7.h"
#include "rmi_ref/osm_cellids_200M_uint64_8.h"
#include "rmi_ref/osm_cellids_200M_uint64_9.h"
#include "rmi_ref/wiki_ts_200M_uint64_0.h"
#include "rmi_ref/wiki_ts_200M_uint64_1.h"
#include "rmi_ref/wiki_ts_200M_uint64_2.h"
#include "rmi_ref/wiki_ts_200M_uint64_3.h"
#include "rmi_ref/wiki_ts_200M_uint64_4.h"
#include "rmi_ref/wiki_ts_200M_uint64_5.h"
#include "rmi_ref/wiki_ts_200M_uint64_6.h"
#include "rmi_ref/wiki_ts_200M_uint64_7.h"
#include "rmi_ref/wiki_ts_200M_uint64_8.h"
#include "rmi_ref/wiki_ts_200M_uint64_9.h"


using key_type = uint64_t;
using namespace std::chrono;

std::size_t s_glob; ///< global size_t variable


/*======================================================================================================================
 * Recursive Model Index
 *====================================================================================================================*/

/**
 * Builds recursive model indexes of different size on @p keys and performs @p n_reps of lookups on @p samples. Writes
 * results including build time, evaluation time, and lookup time to `std::cout`.
 * @param keys on which the index is built
 * @param samples used for measuring the lookup time
 * @param n_reps number of repetitions
 * @param dataset_name name of the dataset
 */
void benchmark_rmi(const std::vector<key_type> &keys,
                   const std::vector<key_type> &samples,
                   const std::size_t n_reps,
                   const std::string dataset_name)
{
    // Set hyperparameters.
    using layer1_type = rmi::LinearSpline;
    using layer2_type = rmi::LinearRegression;

    // Benchmark each configuration.
    for (std::size_t k = 1; k <= 20; ++k) {
        std::size_t budget = (1UL << k) * 1024;

        // Dermine maximum number of layer 2 models for LS->LR NB+MExp.
        auto n_models = (budget - 2 * sizeof(double) - 2 * sizeof(std::size_t)) / (2 * sizeof(double));

        // Build RMI.
        rmi::Rmi<key_type, layer1_type, layer2_type> test_rmi(keys, n_models);

        // Evaluate RMI error.
        auto n_keys = keys.size();
        std::vector<double> log2_errors;
        log2_errors.reserve(n_keys);

        for (std::size_t i = 0; i != n_keys; ++i) {
            auto key = keys.at(i);
            auto pred = test_rmi.search(key).pos;
            auto err = pred > i ? pred - i : i - pred;
            log2_errors.push_back(std::log2(err+1));
        }

        auto mean_log2e = mean(log2_errors);

#define RUN(RMI_TYPE, SEARCH_FN, N_MODELS) \
        { \
            auto search_fn = SEARCH_FN(); \
            \
            /* Perform n_reps runs. */ \
            for (std::size_t rep = 0; rep != n_reps; ++rep) { \
                \
                /* Build time. */ \
                auto start = steady_clock::now(); \
                RMI_TYPE<key_type, layer1_type, layer2_type> rmi(keys, N_MODELS); \
                auto stop = steady_clock::now(); \
                auto build_time = duration_cast<nanoseconds>(stop - start).count(); \
                \
                /* Eval time. */ \
                std::size_t eval_accu = 0; \
                start = steady_clock::now(); \
                for (std::size_t i = 0; i != samples.size(); ++i) { \
                    auto key = samples.at(i); \
                    auto range = rmi.search(key); \
                    eval_accu += range.pos + range.lo + range.hi; \
                } \
                stop = steady_clock::now(); \
                auto eval_time = duration_cast<nanoseconds>(stop - start).count(); \
                s_glob = eval_accu; \
                \
                /* Lookup time. */ \
                std::size_t lookup_accu = 0; \
                start = steady_clock::now(); \
                for (std::size_t i = 0; i != samples.size(); ++i) { \
                    auto key = samples.at(i); \
                    auto range = rmi.search(key); \
                    auto pos = search_fn(keys.begin() + range.lo, keys.begin() + range.hi, keys.begin() + range.pos, key); \
                    lookup_accu += std::distance(keys.begin(), pos); \
                } \
                stop = steady_clock::now(); \
                auto lookup_time = duration_cast<nanoseconds>(stop - start).count(); \
                s_glob = lookup_accu; \
                \
                /* Report results. */ \
                          /* Dataset */ \
                std::cout << dataset_name << ',' \
                          << keys.size() << ',' \
                          /* Index */ \
                          << "RMI-ours" << ',' \
                          << "\"" << #RMI_TYPE << ',' << "layer2_size=" << N_MODELS << "\"" << ',' \
                          << rmi.size_in_bytes() << ',' \
                          /* Experiment */ \
                          << rep << ',' \
                          << samples.size() << ',' \
                          /* Results */ \
                          << build_time << ',' \
                          << eval_time << ',' \
                          << lookup_time << ',' \
                          /* Checksums */ \
                          << eval_accu << ',' \
                          << lookup_accu << std::endl; \
            } /* reps */ \
        }

        // Perform experiment with configuration according to guideline.
        auto threshold = 5.8; // This is hardware-dependent.
        if (mean_log2e < threshold) {
            RUN(rmi::Rmi, ModelBiasedExponentialSearch, n_models)
        } else {
            n_models = (budget - 2 * sizeof(double) - 2 * sizeof(std::size_t)) / (2 * sizeof(double) + sizeof(std::size_t));
            RUN(rmi::RmiLAbs, BinarySearch, n_models)
        }

#undef RUN
    }

}


/*======================================================================================================================
 * ALEX
 *====================================================================================================================*/

/**
 * Builds ALEX of different size on @p keys and performs @p n_reps of lookups on @p samples. Writes results including
 * build time, evaluation time, and lookup time to `std::cout`.
 * @param keys on which the index is built
 * @param samples used for measuring the lookup time
 * @param n_reps number of repetitions
 * @param dataset_name name of the dataset
 */
void benchmark_alex(const std::vector<key_type> &keys,
                    const std::vector<key_type> &samples,
                    const std::size_t n_reps,
                    const std::string dataset_name)
{
    // Set hyperparameters.
    std::size_t min_sparcity = 0;
    std::size_t max_sparcity = 14;

    // Benchmark each configuration.
    for (std::size_t k = min_sparcity; k <= max_sparcity; k++) {
        std::size_t sparcity = 1UL << k;

        // Prepare dataset.
        std::vector<std::pair<key_type, std::size_t>> dataset;
        dataset.reserve(keys.size() / sparcity);
        for (std::size_t i = 0; i != keys.size(); ++i)
            if (i % sparcity == 0) dataset.emplace_back(keys[i], i);

        // Perform n_reps runs.
        for (std::size_t rep = 0; rep != n_reps; ++rep) {

            // Build time.
            auto start = steady_clock::now();
            alex::Alex<key_type, std::size_t> alex;
            alex.bulk_load(dataset.data(), dataset.size());
            auto stop = steady_clock::now();
            auto build_time = duration_cast<nanoseconds>(stop - start).count();

            // Eval time.
            std::size_t eval_accu = 0;
            start = steady_clock::now();
            for (std::size_t i = 0; i != samples.size(); ++i) {
                auto key = samples.at(i);
                auto it = alex.lower_bound(key);
                auto res = it == alex.end() ? keys.size() - 1 : it.payload();
                eval_accu += res;
            }
            stop = steady_clock::now();
            auto eval_time = duration_cast<nanoseconds>(stop - start).count();
            s_glob = eval_accu;

            // Lookup time.
            std::size_t lookup_accu = 0;
            start = steady_clock::now();
            for (std::size_t i = 0; i != samples.size(); ++i) {
                auto key = samples.at(i);
                auto it = alex.lower_bound(key);
                auto res = it == alex.end() ? keys.size() - 1 : it.payload();
                auto lo = res < sparcity - 1 ? 0 : res - (sparcity - 1);
                auto hi = std::min<std::size_t>(keys.size(), res + 1);
                auto pos = std::lower_bound(keys.begin() + lo, keys.begin() + hi, key);
                lookup_accu += std::distance(keys.begin(), pos);
            }
            stop = steady_clock::now();
            auto lookup_time = duration_cast<nanoseconds>(stop - start).count();
            s_glob = lookup_accu;

            // Report results.
                      // Dataset
            std::cout << dataset_name << ','
                      << keys.size() << ','
                      // Index
                      << "ALEX" << ','
                      << "\"sparcity=" << sparcity << "\"" << ','
                      << alex.model_size() + alex.data_size() << ','
                      // Experiment
                      << rep << ','
                      << samples.size() << ','
                      // Results
                      << build_time << ','
                      << eval_time << ','
                      << lookup_time << ','
                      // Checksums
                      << eval_accu << ','
                      << lookup_accu << std::endl;
        } // rep
    } // k
}


/*======================================================================================================================
 * PGM-index
 *====================================================================================================================*/

/**
 * Builds PGM-indexes of different size on @p keys and performs @p n_reps of lookups on @p samples. Writes results
 * including build time, evaluation time, and lookup time to `std::cout`.
 * @param keys on which the index is built
 * @param samples used for measuring the lookup time
 * @param n_reps number of repetitions
 * @param dataset_name name of the dataset
 */
void benchmark_pgm(const std::vector<key_type> &keys,
                   const std::vector<key_type> &samples,
                   const std::size_t n_reps,
                   const std::string dataset_name)
{
#define PGM(EPSILON, EPSILON_RECURSIVE) \
    { \
        /* Set hyperparameters. */ \
        const int epsilon = EPSILON; \
        const int epsilon_recursive = EPSILON_RECURSIVE; \
        \
        /* Perform n_reps runs. */ \
        for (std::size_t rep = 0; rep != n_reps; ++rep) { \
            \
            /* Build time. */ \
            auto start = steady_clock::now(); \
            pgm::PGMIndex<key_type, epsilon, epsilon_recursive> pgm(keys); \
            auto stop = steady_clock::now(); \
            auto build_time = duration_cast<nanoseconds>(stop - start).count(); \
            \
            /* Eval time. */ \
            std::size_t eval_accu = 0; \
            start = steady_clock::now(); \
            for (std::size_t i = 0; i != samples.size(); ++i) { \
                auto key = samples.at(i); \
                auto range = pgm.search(key); \
                eval_accu += range.pos + range.lo + range.hi; \
            } \
            stop = steady_clock::now(); \
            auto eval_time = duration_cast<nanoseconds>(stop - start).count(); \
            s_glob = eval_accu; \
            \
            /* Lookup time. */ \
            std::size_t lookup_accu = 0; \
            start = steady_clock::now(); \
            for (std::size_t i = 0; i != samples.size(); ++i) { \
                auto key = samples.at(i); \
                auto range = pgm.search(key); \
                auto pos = std::lower_bound(keys.begin() + range.lo, keys.begin() + range.hi, key); \
                lookup_accu += std::distance(keys.begin(), pos); \
            } \
            stop = steady_clock::now(); \
            auto lookup_time = duration_cast<nanoseconds>(stop - start).count(); \
            s_glob = lookup_accu; \
            \
            /* Report results. */ \
                      /* Dataset */ \
            std::cout << dataset_name << ',' \
                      << keys.size() << ',' \
                      /* Index */ \
                      << "PGM-index" << ',' \
                      << "\"epsilon=" << epsilon << ",epsilon_recursive=" << epsilon_recursive << "\"" << ',' \
                      << pgm.size_in_bytes() << ',' \
                      /* Experiment */ \
                      << rep << ',' \
                      << samples.size() << ',' \
                      /* Results */ \
                      << build_time << ',' \
                      << eval_time << ',' \
                      << lookup_time << ',' \
                      /* Checksums */ \
                      << eval_accu << ',' \
                      << lookup_accu << std::endl; \
        } \
    }

    PGM(8192, 16)
    PGM(4096, 16)
    PGM(2048, 16)
    PGM(1024, 16)
    PGM(512, 16)
    PGM(256, 16)
    PGM(128, 16)
    PGM(64, 16)
    PGM(32, 16)
    PGM(16, 16)
    PGM(8, 16)
    PGM(4, 16)
    PGM(2, 16)
    PGM(1, 16)

#undef PGM
}


/*======================================================================================================================
 * RadixSpline
 *====================================================================================================================*/

/**
 * Builds RadixSplines of different size on @p keys and performs @p n_reps of lookups on @p samples. Writes results
 * including build time, evaluation time, and lookup time to `std::cout`.
 * @param keys on which the index is built
 * @param samples used for measuring the lookup time
 * @param n_reps number of repetitions
 * @param dataset_name name of the dataset
 */
void benchmark_rs(const std::vector<key_type> &keys,
                  const std::vector<key_type> &samples,
                  const std::size_t n_reps,
                  const std::string dataset_name)
{
    // Set hyperparameters.
    std::vector<std::size_t> radix_bits = { 8, 10, 12, 14, 16, 20, 22, 24, 26, 28 };
    std::vector<std::size_t> max_errors = { 1, 2, 4, 8, 16, 32, 64, 128, 256 };

    // Benchmark each configuration.
    for (auto num_radix_bits : radix_bits) {
        for (auto max_error : max_errors) {

            // Perform n_reps runs.
            for (std::size_t rep = 0; rep != n_reps; ++rep) {

                // Build time.
                auto start = steady_clock::now();
                rs::Builder<key_type> rsb(keys.front(), keys.back(), num_radix_bits, max_error);
                for (const key_type &key : keys) rsb.AddKey(key);
                rs::RadixSpline<key_type> rs = rsb.Finalize();
                auto stop = steady_clock::now();
                auto build_time = duration_cast<nanoseconds>(stop - start).count();

                // Eval time.
                std::size_t eval_accu = 0;
                start = steady_clock::now();
                for (std::size_t i = 0; i != samples.size(); ++i) {
                    auto key = samples.at(i);
                    auto range = rs.GetSearchBound(key);
                    eval_accu += range.begin + range.end;
                }
                stop = steady_clock::now();
                auto eval_time = duration_cast<nanoseconds>(stop - start).count();
                s_glob = eval_accu;

                // Lookup time.
                std::size_t lookup_accu = 0;
                start = steady_clock::now();
                for (std::size_t i = 0; i != samples.size(); ++i) {
                    auto key = samples.at(i);
                    auto range = rs.GetSearchBound(key);
                    auto pos = std::lower_bound(keys.begin() + range.begin, keys.begin() + range.end, key);
                    lookup_accu += std::distance(keys.begin(), pos);
                }
                stop = steady_clock::now();
                auto lookup_time = duration_cast<nanoseconds>(stop - start).count();
                s_glob = lookup_accu;

                // Report results.
                          // Dataset
                std::cout << dataset_name << ','
                          << keys.size() << ','
                          // Index
                          << "RadixSpline" << ','
                          << "\"max_error=" << max_error << ",num_radix_bits=" << num_radix_bits << "\"" << ','
                          << rs.GetSize() << ','
                          // Experiment
                          << rep << ','
                          << samples.size() << ','
                          // Results
                          << build_time << ','
                          << eval_time << ','
                          << lookup_time << ','
                          // Checksums
                          << eval_accu << ','
                          << lookup_accu << std::endl;
            } // rep
        } // max_error
    } // num_radix_bits
}


/*======================================================================================================================
 * Compact Hist-Tree
 *====================================================================================================================*/

/**
 * Builds Compact Hist-Trees of different size on @p keys and performs @p n_reps of lookups on @p samples. Writes
 * results including build time, evaluation time, and lookup time to `std::cout`.
 * @param keys on which the index is built
 * @param samples used for measuring the lookup time
 * @param n_reps number of repetitions
 * @param dataset_name name of the dataset
 */
void benchmark_cht(const std::vector<key_type> &keys,
                   const std::vector<key_type> &samples,
                   const std::size_t n_reps,
                   const std::string dataset_name)
{
    // Set hyperparameters.
    std::vector<std::pair<std::size_t, std::size_t>> configs = {
        {16, 512},
        {32, 512},
        {64, 16}, {64, 32}, {64, 64}, {64, 128}, {64, 256}, {64, 512},
        {128, 16}, {128, 32}, {128, 64}, {128, 128}, {128, 256}, {128, 512},
        {256, 16}, {256, 32}, {256, 64}, {256, 128}, {256, 256}, {256, 512},
        {512, 16}, {512, 32}, {512, 64}, {512, 128}, {512, 256}, {512, 512},
        {1024, 16}, {1024, 32}, {1024, 64}, {1024, 128}, {1024, 256}, {1024, 512},
    };

    // Benchmark each configuration.
    for (auto config : configs) {
        std::size_t num_bins = config.first;
        std::size_t max_error = config.second;

        // Perform n_reps runs.
        for (std::size_t rep = 0; rep != n_reps; ++rep) {

            // Build time.
            auto start = steady_clock::now();
            cht::Builder<key_type> chtb(keys.front(), keys.back(), num_bins, max_error, false, false);
            for (const key_type &key : keys) chtb.AddKey(key);
            cht::CompactHistTree<key_type> cht = chtb.Finalize();
            auto stop = steady_clock::now();
            auto build_time = duration_cast<nanoseconds>(stop - start).count();

            // Eval time.
            std::size_t eval_accu = 0;
            start = steady_clock::now();
            for (std::size_t i = 0; i != samples.size(); ++i) {
                auto key = samples.at(i);
                auto range = cht.GetSearchBound(key);
                eval_accu += range.begin + range.end;
            }
            stop = steady_clock::now();
            auto eval_time = duration_cast<nanoseconds>(stop - start).count();
            s_glob = eval_accu;

            // Lookup time.
            std::size_t lookup_accu = 0;
            start = steady_clock::now();
            for (std::size_t i = 0; i != samples.size(); ++i) {
                auto key = samples.at(i);
                auto range = cht.GetSearchBound(key);
                auto pos = std::lower_bound(keys.begin() + range.begin, keys.begin() + range.end, key);
                lookup_accu += std::distance(keys.begin(), pos);
            }
            stop = steady_clock::now();
            auto lookup_time = duration_cast<nanoseconds>(stop - start).count();
            s_glob = lookup_accu;

            // Report results.
                      // Dataset
            std::cout << dataset_name << ','
                      << keys.size() << ','
                      // Index
                      << "Compact Hist-Tree" << ','
                      << "\"num_bins=" << num_bins << ",max_error=" << max_error << "\"" << ','
                      << cht.GetSize() << ','
                      // Experiment
                      << rep << ','
                      << samples.size() << ','
                      // Results
                      << build_time << ','
                      << eval_time << ','
                      << lookup_time << ','
                      // Checksums
                      << eval_accu << ','
                      << lookup_accu << std::endl;
        } // rep
    } // config
}


/*======================================================================================================================
 * Adaptive Radix Tree
 *====================================================================================================================*/

/**
 * Builds Adaptive Radix Trees of different size on @p keys and performs @p n_reps of lookups on @p samples. Writes
 * results including build time, evaluation time, and lookup time to `std::cout`.
 * @param keys on which the index is built
 * @param samples used for measuring the lookup time
 * @param n_reps number of repetitions
 * @param dataset_name name of the dataset
 */
void benchmark_art(const std::vector<key_type> &keys,
                   const std::vector<key_type> &samples,
                   const std::size_t n_reps,
                   const std::string dataset_name)
{
    // Set hyperparameters.
    std::size_t min_sparcity = 0;
    std::size_t max_sparcity = 14;

    // Benchmark each configuration.
    for (std::size_t k = min_sparcity; k <= max_sparcity; k++) {
        std::size_t sparcity = 1UL << k;

        // Prepare dataset.
        std::vector<art::KeyValue<key_type, std::size_t>> dataset;
        dataset.reserve(keys.size() / sparcity);
        for (std::size_t i = 0; i != keys.size(); ++i)
            dataset.push_back({keys[i], i});

        // Perform n_reps runs.
        for (std::size_t rep = 0; rep != n_reps; ++rep) {

            // Build time.
            auto start = steady_clock::now();
            art::ART art(dataset, sparcity);
            auto stop = steady_clock::now();
            auto build_time = duration_cast<nanoseconds>(stop - start).count();

            // Eval time.
            std::size_t eval_accu = 0;
            start = steady_clock::now();
            for (std::size_t i = 0; i != samples.size(); ++i) {
                auto key = samples.at(i);
                auto range = art.search(key);
                eval_accu += range.first + range.second;
            }
            stop = steady_clock::now();
            auto eval_time = duration_cast<nanoseconds>(stop - start).count();
            s_glob = eval_accu;

            // Lookup time.
            std::size_t lookup_accu = 0;
            start = steady_clock::now();
            for (std::size_t i = 0; i != samples.size(); ++i) {
                auto key = samples.at(i);
                auto range = art.search(key);
                auto pos = std::lower_bound(keys.begin() + range.first, keys.begin() + range.second, key);
                lookup_accu += std::distance(keys.begin(), pos);
            }
            stop = steady_clock::now();
            auto lookup_time = duration_cast<nanoseconds>(stop - start).count();
            s_glob = lookup_accu;

            // Report results.
                      // Dataset
            std::cout << dataset_name << ','
                      << keys.size() << ','
                      // Index
                      << "ART" << ','
                      << "\"sparcity=" << sparcity << "\"" << ','
                      << art.size_in_bytes() << ','
                      // Experiment
                      << rep << ','
                      << samples.size() << ','
                      // Results
                      << build_time << ','
                      << eval_time << ','
                      << lookup_time << ','
                      // Checksums
                      << eval_accu << ','
                      << lookup_accu << std::endl;
        } // rep
    } // sparcity
}


/*======================================================================================================================
 * B-tree
 *====================================================================================================================*/

/**
 * Builds B-trees of different size on @p keys and performs @p n_reps of lookups on @p samples. Writes results including
 * build time, evaluation time, and lookup time to `std::cout`.
 * @param keys on which the index is built
 * @param samples used for measuring the lookup time
 * @param n_reps number of repetitions
 * @param dataset_name name of the dataset
 */
void benchmark_tlx(const std::vector<key_type> &keys,
                   const std::vector<key_type> &samples,
                   const std::size_t n_reps,
                   const std::string dataset_name)
{
    // Set hyperparameters.
    std::size_t min_sparcity = 0;
    std::size_t max_sparcity = 14;

    // Benchmark each configuration.
    for (std::size_t k = min_sparcity; k <= max_sparcity; k++) {
        std::size_t sparcity = 1UL << k;

        // Prepare dataset.
        std::vector<std::pair<key_type, std::size_t>> dataset;
        dataset.reserve(keys.size() / sparcity);
        for (std::size_t i = 0; i != keys.size(); ++i)
            if (i % sparcity == 0) dataset.emplace_back(keys[i], i);

        // Perform n_reps runs.
        for (std::size_t rep = 0; rep != n_reps; ++rep) {

            // Build time.
            auto start = steady_clock::now();
            tlx::btree_multimap<key_type, std::size_t> btree;
            btree.bulk_load(dataset.begin(), dataset.end());
            auto stop = steady_clock::now();
            auto build_time = duration_cast<nanoseconds>(stop - start).count();

            // Eval time.
            std::size_t eval_accu = 0;
            start = steady_clock::now();
            for (std::size_t i = 0; i != samples.size(); ++i) {
                auto key = samples.at(i);
                auto it = btree.lower_bound(key);
                auto res = it == btree.end() ? keys.size() - 1 : it->second;
                eval_accu += res;
            }
            stop = steady_clock::now();
            auto eval_time = duration_cast<nanoseconds>(stop - start).count();
            s_glob = eval_accu;

            // Lookup time.
            std::size_t lookup_accu = 0;
            start = steady_clock::now();
            for (std::size_t i = 0; i != samples.size(); ++i) {
                auto key = samples.at(i);
                auto it = btree.lower_bound(key);
                auto res = it == btree.end() ? keys.size() - 1 : it->second;
                auto lo = res < sparcity - 1 ? 0 : res - (sparcity - 1);
                auto hi = std::min<std::size_t>(keys.size(), res + 1);
                auto pos = std::lower_bound(keys.begin() + lo, keys.begin() + hi, key);
                lookup_accu += std::distance(keys.begin(), pos);
            }
            stop = steady_clock::now();
            auto lookup_time = duration_cast<nanoseconds>(stop - start).count();
            s_glob = lookup_accu;

            // Compute size.
            auto stats = btree.get_stats();
            auto inner_slots = stats.inner_slots;
            auto n_inner_nodes = stats.inner_nodes;
            auto inner_node_size = inner_slots * sizeof(key_type) + (inner_slots + 1) * sizeof(void*); // keys and pointers

            auto leaf_slots = stats.leaf_slots;
            auto n_leaves = stats.leaves;
            auto leaf_size = 2 * sizeof(void*) + leaf_slots * (sizeof(key_type) + sizeof(uint64_t)); // prev/next + data

            double size_in_bytes = inner_node_size * n_inner_nodes + leaf_size * n_leaves;

            // Report results.
                      // Dataset
            std::cout << dataset_name << ','
                      << keys.size() << ','
                      // Index
                      << "B-tree" << ','
                      << "\"sparcity=" << sparcity << "\"" << ','
                      << size_in_bytes << ','
                      // Experiment
                      << rep << ','
                      << samples.size() << ','
                      // Results
                      << build_time << ','
                      << eval_time << ','
                      << lookup_time << ','
                      // Checksums
                      << eval_accu << ','
                      << lookup_accu << std::endl;
        } // rep
    } // k
}


/*======================================================================================================================
 * Recursive Model Index (Reference Implementation)
 *====================================================================================================================*/

/**
 * Loads pre-built recursive model indexes and performs @p n_reps of lookups on @p samples.  Writes results including
 * build time, evaluation time, and lookup time to `std::cout`.
 * @param keys on which the index is built
 * @param samples used for measuring the lookup time
 * @param n_reps number of repetitions
 * @param dataset_name name of the dataset
 */
void benchmark_ref(const std::vector<key_type> &keys,
                   const std::vector<key_type> &samples,
                   const std::size_t n_reps,
                   const std::string dataset_name)
{
#define RMI_DATA_PATH "third_party/RMI/include/rmi_ref/rmi_data"
#define RUN(NAMESPACE) \
    /* Load RMI parameters. */ \
    if (not NAMESPACE::load(RMI_DATA_PATH)) { \
        std::cout << "Could not load RMI." << std::endl; \
        exit(EXIT_FAILURE); \
    } \
    /* Perform n_reps runs. */ \
    for (std::size_t rep = 0; rep != n_reps; ++rep) { \
        \
        /* Build time. */ \
        std::size_t build_time = NAMESPACE::BUILD_TIME_NS; \
        \
        /* Eval time. */ \
        std::size_t eval_accu = 0; \
        std::size_t err = 0; \
        auto start = steady_clock::now(); \
        for (std::size_t i = 0; i != samples.size(); ++i) { \
            auto key = samples.at(i); \
            auto res = NAMESPACE::lookup(key, &err); \
            eval_accu += res + err; \
        } \
        auto stop = steady_clock::now(); \
        auto eval_time = duration_cast<nanoseconds>(stop - start).count(); \
        s_glob = eval_accu; \
        \
        /* Lookup time. */ \
        std::size_t lookup_accu = 0; \
        start = steady_clock::now(); \
        for (std::size_t i = 0; i != samples.size(); ++i) { \
            auto key = samples.at(i); \
            auto res = NAMESPACE::lookup(key, &err); \
            auto lo = res < err  ? 0 : res - err; \
            auto hi = res + err >= keys.size() ? keys.size() : res + err; \
            auto pos = std::lower_bound(keys.begin() + lo, keys.begin() + hi, key); \
            lookup_accu += std::distance(keys.begin(), pos); \
        } \
        stop = steady_clock::now(); \
        auto lookup_time = duration_cast<nanoseconds>(stop - start).count(); \
        s_glob = lookup_accu; \
        \
        /* Get size. */ \
        std::size_t size_in_bytes = NAMESPACE::RMI_SIZE; \
        \
        /* Report results. */ \
                  /* Dataset */ \
        std::cout << dataset_name << ',' \
                  << keys.size() << ',' \
                  /* Index */ \
                  << "RMI-ref" << ',' \
                  << #NAMESPACE << ',' \
                  << size_in_bytes << ',' \
                  /* Experiment */ \
                  << rep << ',' \
                  << samples.size() << ',' \
                  /* Results */ \
                  << build_time << ',' \
                  << eval_time << ',' \
                  << lookup_time << ',' \
                  /* Checksums */ \
                  << eval_accu << ',' \
                  << lookup_accu << std::endl; \
    } /* rep */ \
    NAMESPACE::cleanup(); \

    if (dataset_name == "books_200M_uint64") {
        RUN(books_200M_uint64_0)
        RUN(books_200M_uint64_1)
        RUN(books_200M_uint64_2)
        RUN(books_200M_uint64_3)
        RUN(books_200M_uint64_4)
        RUN(books_200M_uint64_5)
        RUN(books_200M_uint64_6)
        RUN(books_200M_uint64_7)
        RUN(books_200M_uint64_8)
        RUN(books_200M_uint64_9)
    } else if (dataset_name == "fb_200M_uint64") {
        RUN(fb_200M_uint64_0)
        RUN(fb_200M_uint64_1)
        RUN(fb_200M_uint64_2)
        RUN(fb_200M_uint64_3)
        RUN(fb_200M_uint64_4)
        RUN(fb_200M_uint64_5)
        RUN(fb_200M_uint64_6)
        RUN(fb_200M_uint64_7)
        RUN(fb_200M_uint64_8)
        RUN(fb_200M_uint64_9)
    } else if (dataset_name == "osm_cellids_200M_uint64") {
        RUN(osm_cellids_200M_uint64_0)
        RUN(osm_cellids_200M_uint64_1)
        RUN(osm_cellids_200M_uint64_2)
        RUN(osm_cellids_200M_uint64_3)
        RUN(osm_cellids_200M_uint64_4)
        RUN(osm_cellids_200M_uint64_5)
        RUN(osm_cellids_200M_uint64_6)
        RUN(osm_cellids_200M_uint64_7)
        RUN(osm_cellids_200M_uint64_8)
        RUN(osm_cellids_200M_uint64_9)
    } else if (dataset_name == "wiki_ts_200M_uint64") {
        RUN(wiki_ts_200M_uint64_0)
        RUN(wiki_ts_200M_uint64_1)
        RUN(wiki_ts_200M_uint64_2)
        RUN(wiki_ts_200M_uint64_3)
        RUN(wiki_ts_200M_uint64_4)
        RUN(wiki_ts_200M_uint64_5)
        RUN(wiki_ts_200M_uint64_6)
        RUN(wiki_ts_200M_uint64_7)
        RUN(wiki_ts_200M_uint64_8)
        RUN(wiki_ts_200M_uint64_9)
    } else {
        std::cerr << "Reference implementation RMI not pre-trained for given dataset. Skipping." << std::endl;
        return;
    }
#undef RUN
#undef RMI_DATA_PATH
}


/*======================================================================================================================
 * Binary search
 *====================================================================================================================*/

/**
 * Performs lookups of @p samples on @p keys using binary search
 * @param keys that are searched
 * @param samples used for measuring the lookup time
 * @param n_reps number of repetitions
 * @param dataset_name name of the dataset
 */
void benchmark_bin(const std::vector<key_type> &keys,
                   const std::vector<key_type> &samples,
                   const std::size_t n_reps,
                   const std::string dataset_name)
{
    // Perform n_reps runs.
    for (std::size_t rep = 0; rep != n_reps; ++rep) {

        // Build time.
        std::size_t build_time = 0;

        // Eval time.
        std::size_t eval_accu = 0;
        std::size_t eval_time = 0;

        // Lookup time.
        std::size_t lookup_accu = 0;
        auto start = steady_clock::now();
        for (std::size_t i = 0; i != samples.size(); ++i) {
            auto key = samples.at(i);
            auto pos = std::lower_bound(keys.begin(), keys.end(), key);
            lookup_accu += std::distance(keys.begin(), pos);
        }
        auto stop = steady_clock::now();
        auto lookup_time = duration_cast<nanoseconds>(stop - start).count();
        s_glob = lookup_accu;

        // Compute size.
        double size_in_bytes = 0.f;

        // Report results.
                  // Dataset
        std::cout << dataset_name << ','
                  << keys.size() << ','
                  // Index
                  << "\"Binary search\"" << ','
                  << "\"\"" << ','
                  << size_in_bytes << ','
                  // Experiment
                  << rep << ','
                  << samples.size() << ','
                  // Results
                  << build_time << ','
                  << eval_time << ','
                  << lookup_time << ','
                  // Checksums
                  << eval_accu << ','
                  << lookup_accu << std::endl;
    } // rep
}

/**
 * Performs an index comparison in terms of build time, evaluation time, and lookup time.
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

   program.add_argument("-n", "--n_reps") // Make this optional
        .help("number of experiment repetitions")
        .default_value(std::size_t(3))
        .action([](const std::string &s) { return std::stoul(s); });

    program.add_argument("-s", "--n_samples") // Make this optional
        .help("number of sampled lookup keys")
        .default_value(std::size_t(1'000'000))
        .action([](const std::string &s) { return std::stoul(s); });

    program.add_argument("--header")
        .help("output csv header")
        .default_value(false)
        .implicit_value(true);

    program.add_argument("--rmi")
        .help("run benchmark on Recursive Model Index")
        .default_value(false)
        .implicit_value(true);

    program.add_argument("--alex")
        .help("run benchmark on ALEX")
        .default_value(false)
        .implicit_value(true);

    program.add_argument("--pgm")
        .help("run benchmark on PGM-index")
        .default_value(false)
        .implicit_value(true);

    program.add_argument("--rs")
        .help("run benchmark on RadixSpline")
        .default_value(false)
        .implicit_value(true);

    program.add_argument("--cht")
        .help("run benchmark on Compact Hist-Tree")
        .default_value(false)
        .implicit_value(true);

    program.add_argument("--art")
        .help("run benchmark on Adaptive Radix Tree")
        .default_value(false)
        .implicit_value(true);

    program.add_argument("--tlx")
        .help("run benchmark on TLX B-tree")
        .default_value(false)
        .implicit_value(true);

    program.add_argument("--ref")
        .help("run benchmark on reference implementation of Recursive Model Index")
        .default_value(false)
        .implicit_value(true);

    program.add_argument("--bin")
        .help("run benchmark on binary search")
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

    // Output header.
    if (program["--header"]  == true)
        std::cout << "dataset,"
                  << "n_keys,"
                  << "index,"
                  << "config,"
                  << "size_in_bytes,"
                  << "rep,"
                  << "n_samples,"
                  << "build_time,"
                  << "eval_time,"
                  << "lookup_time,"
                  << "eval_accu,"
                  << "lookup_accu"
                  << std::endl;

    // Run benchmarks.
    if (program["--rmi"]  == true) benchmark_rmi(keys, samples, n_reps, dataset_name);
    if (program["--alex"] == true) benchmark_alex(keys, samples, n_reps, dataset_name);
    if (program["--pgm"]  == true) benchmark_pgm(keys, samples, n_reps, dataset_name);
    if (program["--rs"]   == true) benchmark_rs(keys, samples, n_reps, dataset_name);
    if (program["--cht"]  == true) benchmark_cht(keys, samples, n_reps, dataset_name);
    if (program["--art"]  == true) benchmark_art(keys, samples, n_reps, dataset_name);
    if (program["--tlx"]  == true) benchmark_tlx(keys, samples, n_reps, dataset_name);
    if (program["--ref"]  == true) benchmark_ref(keys, samples, n_reps, dataset_name);
    if (program["--bin"]  == true) benchmark_bin(keys, samples, n_reps, dataset_name);

    exit(EXIT_SUCCESS);
}
