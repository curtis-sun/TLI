#pragma once

#include <algorithm>
#include <fstream>
#include <iostream>
#include <limits>
#include <numeric>
#include <sstream>
#include <type_traits>
#include <vector>


/*======================================================================================================================
 * Bit Functions
 *====================================================================================================================*/

/**
 * Computes the amount of bits needed to represent unsigned value @p n.
 * @tparam Numeric the type of the value
 * @param n the value
 * @return the bit-width of the value
 */
template<typename Numeric>
uint8_t bit_width(Numeric n)
{
    static_assert(std::is_unsigned<Numeric>::value, "not defined for signed integral types");

    // Count leading zeros.
    int lz;
    if constexpr (std::is_same_v<Numeric, unsigned>) {
        lz = __builtin_clz(n);
    } else if constexpr (std::is_same_v<Numeric, unsigned long>) {
        lz = __builtin_clzl(n);
    } else if constexpr (std::is_same_v<Numeric, unsigned long long>) {
        lz = __builtin_clzll(n);
    } else {
        static_assert(sizeof(Numeric) > sizeof(unsigned long long), "unsupported width of integral type");
    }

    return sizeof(Numeric) * 8 - lz;
}

/**
 * Computes the length of the common prefix of two numeric values @p v1 and @p v2.
 * @tparam Numeric the type of the values
 * @param v1 the first value
 * @param v2 the second value
 * @return the length of the common prefix
 */
template<typename Numeric>
uint8_t common_prefix_width(Numeric v1, Numeric v2)
{
    Numeric Xor = v1 ^ v2; // bit-wise xor

    if constexpr (sizeof(Numeric) <= sizeof(unsigned)) {
        return __builtin_clz(Xor);
    } else if constexpr (sizeof(Numeric) <= sizeof(unsigned long)) {
        return __builtin_clzl(Xor);
    } else if constexpr (sizeof(Numeric) <= sizeof(unsigned long long)) {
        return __builtin_clzll(Xor);
    } else {
        static_assert(sizeof(Numeric) > sizeof(unsigned long long), "unsupported width of integral type");
    }
}


/*======================================================================================================================
 * String Functions
 *====================================================================================================================*/

/**
 * Splits @p str at each occurence of @p delimiter.
 * @param str the string to be split
 * @param delimiter the delimiter to split the string at
 * @return vector of substrings
 */
std::vector<std::string> split(const std::string &str, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream token_stream(str);
    while (std::getline(token_stream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}


/*======================================================================================================================
 * Arithmetic Functions
 *====================================================================================================================*/

/**
 * Computes the arithmetic mean of a vector @p v of numeric values.
 * @param v vector of numeric values
 * @return arithmetic mean
 */
template<typename Numeric>
double mean(std::vector<Numeric> &v)
{
    double sum = std::accumulate(v.begin(), v.end(), 0.0);
    return sum / v.size();
}

/**
 * Computes the standard deviation of the mean of vector @p of numeric values.
 * @param v vector of numeric values
 * @return standard deviation
 */
template<typename Numeric>
double stdev(std::vector<Numeric> &v) {
    double mean = ::mean<Numeric>(v);
    double sq_sum = std::inner_product(v.begin(), v.end(), v.begin(), 0.0);
    return std::sqrt(sq_sum / v.size() - mean * mean);
}

/**
 * Computes the median of vector @p v of numeric values.
 * @param v vector of numeric values
 * @return median
 */
template<typename Numeric>
Numeric median(std::vector<Numeric> &v)
{
    std::size_t n = v.size() / 2;
    std::nth_element(v.begin(), v.begin()+n, v.end());
    return v.at(n);
}

/**
 * Computes the minimum of a vector @p v of numeric values.
 * @param v vector of numeric values
 * @return minimum
 */
template<typename Numeric>
Numeric min(std::vector<Numeric> &v)
{
    return *std::min_element(v.begin(), v.end());
}

/**
 * Computes the maximum of a vector @p v of numeric values.
 * @param v vector of numeric values
 * @return maximum
 */
template<typename Numeric>
Numeric max(std::vector<Numeric> &v)
{
    return *std::max_element(v.begin(), v.end());
}


/*======================================================================================================================
 * Dataset Functions
 *====================================================================================================================*/

/**
 * Reads a dataset file @p filename in binary format and writes keys to vector.
 * @tparam Key the type of the key
 * @param filename name of the dataset file
 * @return vector of keys
 */
template<typename Key>
std::vector<Key> load_data(const std::string &filename) {
    using key_type = Key;

    // Open file.
    std::ifstream in(filename, std::ios::binary);
    if (!in.is_open()) {
        std::cerr << "Could not load " << filename << '.' << std::endl;
        exit(EXIT_FAILURE);
    }

    // Read number of keys.
    uint64_t n_keys;
    in.read(reinterpret_cast<char*>(&n_keys), sizeof(uint64_t));

    // Initialize vector.
    std::vector<key_type> data;
    data.resize(n_keys);

    // Read keys.
    in.read(reinterpret_cast<char*>(data.data()), n_keys * sizeof(key_type));
    in.close();

    return data;
}
