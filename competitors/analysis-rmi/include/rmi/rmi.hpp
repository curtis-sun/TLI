#pragma once

#include <algorithm>
#include <vector>


namespace rmi {

/**
 * Struct to hold the approximated position and error bounds returned by the index.
 */
struct Approx {
    std::size_t pos; ///< The estimated position of the key.
    std::size_t lo;  ///< The lower bound of the search range.
    std::size_t hi;  ///< The upper bound of the search range.
};

/**
 * This is a reimplementation of a two-layer recursive model index (RMI) supporting a variety of (monotonic) models.
 * RMIs were invented by Kraska et al. (https://dl.acm.org/doi/epdf/10.1145/3183713.3196909).
 *
 * Note that this is the base class which does not provide error bounds.
 *
 * @tparam Key the type of the keys to be indexed
 * @tparam Layer1 the type of the model used in layer1
 * @tparam Layer2 the type of the models used in layer2
 */
template<typename Key, typename Layer1, typename Layer2>
class Rmi
{
    using key_type = Key;
    using layer1_type = Layer1;
    using layer2_type = Layer2;

    protected:
    std::size_t n_keys_;      ///< The number of keys the index was built on.
    std::size_t layer2_size_; ///< The number of models in layer2.
    layer1_type l1_;          ///< The layer1 model.
    layer2_type *l2_;         ///< The array of layer2 models.

    public:
    /**
     * Default constructor.
     */
    Rmi() = default;

    /**
     * Builds the index with @p layer2_size models in layer2 on the sorted @p keys.
     * @param keys vector of sorted keys to be indexed
     * @param layer2_size the number of models in layer2
     */
    Rmi(const std::vector<key_type> &keys, const std::size_t layer2_size)
        : Rmi(keys.begin(), keys.end(), layer2_size) { }

    /**
     * Builds the index with @p layer2_size models in layer2 on the sorted keys in the range [first, last).
     * @param first, last iterators that define the range of sorted keys to be indexed
     * @param layer2_size the number of models in layer2
     */
    template<typename RandomIt>
    Rmi(RandomIt first, RandomIt last, const std::size_t layer2_size)
        : n_keys_(std::distance(first, last))
        , layer2_size_(layer2_size)
    {
        // Train layer1.
        l1_ = layer1_type(first, last, 0, static_cast<double>(layer2_size) / n_keys_); // train with compression

        // Train layer2.
        l2_ = new layer2_type[layer2_size];
        std::size_t segment_start = 0;
        std::size_t segment_id = 0;
        // Assign each key to its segment.
        for (std::size_t i = 0; i != n_keys_; ++i) {
            auto pos = first + i;
            std::size_t pred_segment_id = get_segment_id(*pos);
            // If a key is assigned to a new segment, all models must be trained up to the new segment.
            if (pred_segment_id > segment_id) {
                new (&l2_[segment_id]) layer2_type(first + segment_start, pos, segment_start);
                for (std::size_t j = segment_id + 1; j < pred_segment_id; ++j) {
                    new (&l2_[j]) layer2_type(pos - 1, pos, i - 1); // train other models on last key in previous segment
                }
                segment_id = pred_segment_id;
                segment_start = i;
            }
        }
        // Train remaining models.
        new (&l2_[segment_id]) layer2_type(first + segment_start, last, segment_start);
        for (std::size_t j = segment_id + 1; j < layer2_size; ++j) {
            new (&l2_[j]) layer2_type(last - 1, last, n_keys_ - 1); // train remaining models on last key
        }
    }

    /**
     * Destructor.
     */
    ~Rmi() { delete[] l2_; }

    /**
     * Returns the id of the segment @p key belongs to.
     * @param key to get segment id for
     * @return segment id of the given key
     */
    std::size_t get_segment_id(const key_type key) const {
        return std::clamp<double>(l1_.predict(key), 0, layer2_size_ - 1);
    }

    /**
     * Returns a position estimate and search bounds for a given key.
     * @param key to search for
     * @return position estimate and search bounds
     */
    Approx search(const key_type key) const {
        auto segment_id = get_segment_id(key);
        std::size_t pred = std::clamp<double>(l2_[segment_id].predict(key), 0, n_keys_ - 1);
        return {pred, 0, n_keys_};
    }

    /**
     * Returns the number of keys the index was built on.
     * @return the number of keys the index was built on
     */
    std::size_t n_keys() const { return n_keys_; }

    /**
     * Returns the number of models in layer2.
     * @return the number of models in layer2
     */
    std::size_t layer2_size() const { return layer2_size_; }

    /**
     * Returns the size of the index in bytes.
     * @return index size in bytes
     */
    std::size_t size_in_bytes() {
        return l1_.size_in_bytes() + layer2_size_ * l2_[0].size_in_bytes() + sizeof(n_keys_) + sizeof(layer2_size_);
    }
};


/**
 * Recursive model index with global absolute bounds.
 */
template<typename Key, typename Layer1, typename Layer2>
class RmiGAbs : public Rmi<Key, Layer1, Layer2>
{
    using base_type = Rmi<Key, Layer1, Layer2>;
    using key_type = Key;
    using layer1_type = Layer1;
    using layer2_type = Layer2;

    protected:
    std::size_t error_; ///< The error bound of the layer2 models.

    public:
    /**
     * Default constructor.
     */
    RmiGAbs() = default;

    /**
     * Builds the index with @p layer2_size models in layer2 on the sorted @p keys.
     * @param keys vector of sorted keys to be indexed
     * @param layer2_size the number of models in layer2
     */
    RmiGAbs(const std::vector<key_type> &keys, const std::size_t layer2_size)
        : RmiGAbs(keys.begin(), keys.end(), layer2_size) { }

    /**
     * Builds the index with @p layer2_size models in layer2 on the sorted keys in the range [first, last).
     * @param first, last iterators that define the range of sorted keys to be indexed
     * @param layer2_size the number of models in layer2
     */
    template<typename RandomIt>
    RmiGAbs(RandomIt first, RandomIt last, const std::size_t layer2_size) : base_type(first, last, layer2_size) {
        // Compute global absolute errror bounds.
        error_ = 0;
        for (std::size_t i = 0; i != base_type::n_keys_; ++i) {
            key_type key = *(first + i);
            std::size_t segment_id = base_type::get_segment_id(key);
            std::size_t pred = std::clamp<double>(base_type::l2_[segment_id].predict(key), 0, base_type::n_keys_ - 1);
            if (pred > i) { // overestimation
                error_ = std::max(error_, pred - i);
            } else { // underestimation
                error_ = std::max(error_, i - pred);
            }
        }
    }

    /**
     * Returns a position estimate and search bounds for a given key.
     * @param key to search for
     * @return position estimate and search bounds
     */
    Approx search(const key_type key) const {
        auto segment_id = base_type::get_segment_id(key);
        std::size_t pred = std::clamp<double>(base_type::l2_[segment_id].predict(key), 0, base_type::n_keys_ - 1);
        std::size_t lo = pred > error_ ? pred - error_ : 0;
        std::size_t hi = std::min(pred + error_ + 1, base_type::n_keys_);
        return {pred, lo, hi};
    }

    /**
     * Returns the size of the index in bytes.
     * @return index size in bytes
     */
    std::size_t size_in_bytes() { return base_type::size_in_bytes() + sizeof(error_); }
};


/**
 * Recursive model index with global individual bounds.
 */
template<typename Key, typename Layer1, typename Layer2>
class RmiGInd : public Rmi<Key, Layer1, Layer2>
{
    using base_type = Rmi<Key, Layer1, Layer2>;
    using key_type = Key;
    using layer1_type = Layer1;
    using layer2_type = Layer2;

    protected:
    std::size_t error_lo_; ///< The lower error bound of the layer2 models.
    std::size_t error_hi_; ///< The upper error bound of the layer2 models.

    public:
    /**
     * Default constructor.
     */
    RmiGInd() = default;

    /**
     * Builds the index with @p layer2_size models in layer2 on the sorted @p keys.
     * @param keys vector of sorted keys to be indexed
     * @param layer2_size the number of models in layer2
     */
    RmiGInd(const std::vector<key_type> &keys, const std::size_t layer2_size)
        : RmiGInd(keys.begin(), keys.end(), layer2_size) { }

    /**
     * Builds the index with @p layer2_size models in layer2 on the sorted keys in the range [first, last).
     * @param first, last iterators that define the range of sorted keys to be indexed
     * @param layer2_size the number of models in layer2
     */
    template<typename RandomIt>
    RmiGInd(RandomIt first, RandomIt last, const std::size_t layer2_size) : base_type(first, last, layer2_size) {
        // Compute global absolute errror bounds.
        error_lo_ = 0;
        error_hi_ = 0;
        for (std::size_t i = 0; i != base_type::n_keys_; ++i) {
            key_type key = *(first + i);
            std::size_t segment_id = base_type::get_segment_id(key);
            std::size_t pred = std::clamp<double>(base_type::l2_[segment_id].predict(key), 0, base_type::n_keys_ - 1);
            if (pred > i) { // overestimation
                error_lo_ = std::max(error_lo_, pred - i);
            } else { // underestimation
                error_hi_ = std::max(error_hi_, i - pred);
            }
        }
    }

    /**
     * Returns a position estimate and search bounds for a given key.
     * @param key to search for
     * @return position estimate and search bounds
     */
    Approx search(const key_type key) const {
        auto segment_id = base_type::get_segment_id(key);
        std::size_t pred = std::clamp<double>(base_type::l2_[segment_id].predict(key), 0, base_type::n_keys_ - 1);
        std::size_t lo = pred > error_lo_ ? pred - error_lo_ : 0;
        std::size_t hi = std::min(pred + error_hi_ + 1, base_type::n_keys_);
        return {pred, lo, hi};
    }

    /**
     * Returns the size of the index in bytes.
     * @return index size in bytes
     */
    std::size_t size_in_bytes() { return base_type::size_in_bytes() + sizeof(error_lo_) + sizeof(error_hi_); }
};


/**
 * Recursive model index with local absolute bounds.
 */
template<typename Key, typename Layer1, typename Layer2>
class RmiLAbs : public Rmi<Key, Layer1, Layer2>
{
    using base_type = Rmi<Key, Layer1, Layer2>;
    using key_type = Key;
    using layer1_type = Layer1;
    using layer2_type = Layer2;

    protected:
    std::vector<std::size_t> errors_; ///< The error bounds of the layer2 models.

    public:
    /**
     * Default constructor.
     */
    RmiLAbs() = default;

    /**
     * Builds the index with @p layer2_size models in layer2 on the sorted @p keys.
     * @param keys vector of sorted keys to be indexed
     * @param layer2_size the number of models in layer2
     */
    RmiLAbs(const std::vector<key_type> &keys, const std::size_t layer2_size)
        : RmiLAbs(keys.begin(), keys.end(), layer2_size) { }

    /**
     * Builds the index with @p layer2_size models in layer2 on the sorted keys in the range [first, last).
     * @param first, last iterators that define the range of sorted keys to be indexed
     * @param layer2_size the number of models in layer2
     */
    template<typename RandomIt>
    RmiLAbs(RandomIt first, RandomIt last, const std::size_t layer2_size) : base_type(first, last, layer2_size) {
        // Compute local absolute errror bounds.
        errors_ = std::vector<std::size_t>(layer2_size);
        for (std::size_t i = 0; i != base_type::n_keys_; ++i) {
            key_type key = *(first + i);
            std::size_t segment_id = base_type::get_segment_id(key);
            std::size_t pred = std::clamp<double>(base_type::l2_[segment_id].predict(key), 0, base_type::n_keys_ - 1);
            if (pred > i) { // overestimation
                errors_[segment_id] = std::max(errors_[segment_id], pred - i);
            } else { // underestimation
                errors_[segment_id] = std::max(errors_[segment_id], i - pred);
            }
        }
    }

    /**
     * Returns a position estimate and search bounds for a given key.
     * @param key to search for
     * @return position estimate and search bounds
     */
    Approx search(const key_type key) const {
        auto segment_id = base_type::get_segment_id(key);
        std::size_t pred = std::clamp<double>(base_type::l2_[segment_id].predict(key), 0, base_type::n_keys_ - 1);
        std::size_t err = errors_[segment_id];
        std::size_t lo = pred > err ? pred - err : 0;
        std::size_t hi = std::min(pred + err + 1, base_type::n_keys_);
        return {pred, lo, hi};
    }

    /**
     * Returns the size of the index in bytes.
     * @return index size in bytes
     */
    std::size_t size_in_bytes() { return base_type::size_in_bytes() + errors_.size() * sizeof(errors_.front()); }
};


/**
 * Recursive model index with local individual bounds.
 */
template<typename Key, typename Layer1, typename Layer2>
class RmiLInd : public Rmi<Key, Layer1, Layer2>
{
    using base_type = Rmi<Key, Layer1, Layer2>;
    using key_type = Key;
    using layer1_type = Layer1;
    using layer2_type = Layer2;

    protected:
    /**
     * Struct to store a lower and an upper error bound.
     */
    struct bounds {
        std::size_t lo; ///< The lower error bound.
        std::size_t hi; ///< The upper error bound.

        /**
         * Default constructor.
         */
        bounds() : lo(0), hi(0) { }
    };

    std::vector<bounds> errors_; ///< The error bounds of the layer2 models.

    public:
    /**
     * Default constructor.
     */
    RmiLInd() = default;

    /**
     * Builds the index with @p layer2_size models in layer2 on the sorted @p keys.
     * @param keys vector of sorted keys to be indexed
     * @param layer2_size the number of models in layer2
     */
    RmiLInd(const std::vector<key_type> &keys, const std::size_t layer2_size)
        : RmiLInd(keys.begin(), keys.end(), layer2_size) { }

    /**
     * Builds the index with @p layer2_size models in layer2 on the sorted keys in the range [first, last).
     * @param first, last iterators that define the range of sorted keys to be indexed
     * @param layer2_size the number of models in layer2
     */
    template<typename RandomIt>
    RmiLInd(RandomIt first, RandomIt last, const std::size_t layer2_size) : base_type(first, last, layer2_size) {
        // Compute local individual errror bounds.
        errors_ = std::vector<bounds>(layer2_size);
        for (std::size_t i = 0; i != base_type::n_keys_; ++i) {
            key_type key = *(first + i);
            std::size_t segment_id = base_type::get_segment_id(key);
            std::size_t pred = std::clamp<double>(base_type::l2_[segment_id].predict(key), 0, base_type::n_keys_ - 1);
            if (pred > i) { // overestimation
                std::size_t &lo = errors_[segment_id].lo;
                lo = std::max(lo, pred - i);
            } else { // underestimation
                std::size_t &hi = errors_[segment_id].hi;
                hi = std::max(hi, i - pred);
            }
        }
    }

    /**
     * Returns a position estimate and search bounds for a given key.
     * @param key to search for
     * @return position estimate and search bounds
     */
    Approx search(const key_type key) const {
        auto segment_id = base_type::get_segment_id(key);
        std::size_t pred = std::clamp<double>(base_type::l2_[segment_id].predict(key), 0, base_type::n_keys_ - 1);
        bounds err = errors_[segment_id];
        std::size_t lo = pred > err.lo ? pred - err.lo : 0;
        std::size_t hi = std::min(pred + err.hi + 1, base_type::n_keys_);
        return {pred, lo, hi};
    }

    /**
     * Returns the size of the index in bytes.
     * @return index size in bytes
     */
    std::size_t size_in_bytes() { return base_type::size_in_bytes() + errors_.size() * sizeof(errors_.front()); }
};

} // namespace rmi
