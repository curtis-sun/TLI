#pragma once

#include <algorithm>
#include <vector>


/**
 * Functor for performing linear search.
 */
struct LinearSearch {
    /**
     * Performs linear search in the interval [first,last) to find the first element that is not less than @t value.
     * @tparam InputIt input iterator type
     * @tparam T type of searched value
     * @param first, last iterators defining the partially-ordered range to examine
     * @param pred iterator to the predicted position (ignored)
     * @param value value to compare the elements to
     * @return iterator to the first element that is not less than @p value
     */
    template<typename InputIt, typename T>
    InputIt operator()(InputIt first, InputIt last, InputIt pred, const T &value) {
        InputIt runner = first;
        for (; runner != last; ++runner)
            if (*runner >= value) return runner;
        return last;
    }
};


/**
 * Functor for performing model-biased linear search.
 */
struct ModelBiasedLinearSearch {
    /**
     * Performs model-biased linear search either in the interval [first,pred) or [pred, last) to find the first element
     * that is not less than @t value.
     * @tparam InputIt input iterator type
     * @tparam T type of searched value
     * @param first, last iterators defining the partially-ordered range to examine
     * @param pred iterator to the predicted position
     * @param value value to compare the elements to
     * @return iterator to the first element that is not less than @p value
     */
    template<typename InputIt, typename T>
    InputIt operator()(InputIt first, InputIt last, InputIt pred, const T &value) {
        InputIt runner = pred;
        if (*runner < value) {
            for (; runner < last; ++runner) // search right side
                if (*runner >= value) return runner;
            return last;
        } else {
            for (; runner >= first; --runner)// search left side
                if (*runner < value) return ++runner;
            return first;
        }
    }
};


/**
 * Functor for performing binary search.
 */
struct BinarySearch {
    /**
     * Performs binary search in the interval [first,last) to find the first element that is not less than @t value.
     * @tparam InputIt input iterator type
     * @tparam T type of searched value
     * @param first, last iterators defining the partially-ordered range to examine
     * @param pred iterator to the predicted position (ignored)
     * @param value value to compare the elements to
     * @return iterator to the first element that is not less than @p value
     */
    template<typename InputIt, typename T>
    InputIt operator()(InputIt first, InputIt last, InputIt pred, const T &value) {
        return std::lower_bound(first, last, value);
    }
};


/**
 * Functor for performing model-biased binary search.
 */
struct ModelBiasedBinarySearch {
    /**
     * Performs model-biased binary search either in the interval [first,pred) or [pred, last) to find the first element
     * that is not less than @t value.
     * @tparam InputIt input iterator type
     * @tparam T type of searched value
     * @param first, last iterators defining the partially-ordered range to examine
     * @param pred iterator to the predicted position
     * @param value value to compare the elements to
     * @return iterator to the first element that is not less than @p value
     */
    template<typename InputIt, typename T>
    InputIt operator()(InputIt first, InputIt last, InputIt pred, const T &value) {
        if (*pred < value) return std::lower_bound(pred, last, value); // search right side
        else return std::lower_bound(first, pred, value); // search left side
    }
};


/**
 * Functor for performing exponential search.
 * TODO: might need fix for datasets containing duplicates.
 */
struct ExponentialSearch {
    /**
     * Performs exponential search in the interval [first,last) to find the first element that is not less than @t
     * value.
     * @tparam InputIt input iterator type
     * @tparam T type of searched value
     * @param first, last iterators defining the partially-ordered range to examine
     * @param pred iterator to the predicted position (ignored)
     * @param value value to compare the elements to
     * @return iterator to the first element that is not less than @p value
     */
    template<typename InputIt, typename T>
    InputIt operator()(InputIt first, InputIt last, InputIt pred, const T &value) {
        if (*first >= value) return first;
        std::size_t bound = 1;
        InputIt prev = first;
        InputIt curr = prev + bound;
        while (curr < last and *curr < value) {
            bound *= 2;
            prev = curr;
            curr += bound;
        }
        return std::lower_bound(prev, std::min(curr + 1, last), value);
    }
};


/**
 * Functor for performing model-biased exponential search.
 * TODO: might need fix for datasets containing duplicates.
 */
struct ModelBiasedExponentialSearch {
    /**
     * Performs model-biased exponential search either in the interval [first,pred) or [pred, last) to find the first
     * element that is not less than @t value.
     * @tparam InputIt input iterator type
     * @tparam T type of searched value
     * @param first, last iterators defining the partially-ordered range to examine
     * @param pred iterator to the predicted position
     * @param value value to compare the elements to
     * @return iterator to the first element that is not less than @p value
     */
    template<typename InputIt, typename T>
    InputIt operator()(InputIt first, InputIt last, InputIt pred, const T &value) {
        if (*pred < value) { // search right side
            std::size_t bound = 1;
            InputIt prev = pred;
            InputIt curr = prev + bound;
            while (curr < last and *curr < value) {
                bound *= 2;
                prev = curr;
                curr += bound;
            }
            return std::lower_bound(prev, std::min(curr + 1, last), value);
        } else { // search left side
            std::size_t bound = 1;
            InputIt prev = pred;
            InputIt curr = prev - bound;
            while (curr > first and *curr > value) {
                bound *= 2;
                prev = curr;
                curr -= bound;
            }
            return std::lower_bound(std::max(first, curr), prev, value);
        }
    }
};
