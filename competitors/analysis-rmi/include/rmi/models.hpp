#pragma once

#include <cmath>
#include <x86intrin.h>

#include "rmi/util/fn.hpp"

namespace rmi {

/**
 * A model that fits a linear segment from the first first to the last data point.
 *
 * We assume that x-values are sorted in ascending order and y-values are handed implicitly where @p offset and @p
 * offset + distance(first, last) are the first and last y-value, respectively. The y-values can be scaled by
 * providing a @p compression_factor.
 */
class LinearSpline
{
    private:
    double slope_;     ///< The slope of the linear segment.
    double intercept_; ///< The y-intercept of the lienar segment.

    public:
    /**
     * Default contructor.
     */
    LinearSpline() = default;

    /**
     * Builds a linaer segment between the first and last data point.
     * @param first, last iterators to the first and last x-value the linear segment is fit on
     * @param offset first y-value the linear segment is fit on
     * @param compression_factor by which the y-values are scaled
     */
    template<typename RandomIt>
    LinearSpline(RandomIt first, RandomIt last, std::size_t offset = 0, double compression_factor = 1.f) {
        std::size_t n = std::distance(first, last);

        if (n == 0) {
            slope_ = 0.f;
            intercept_ = 0.f;
            return;
        }
        if (n == 1) {
            slope_ = 0.f;
            intercept_ = static_cast<double>(offset) * compression_factor;
            return;
        }

        double numerator = static_cast<double>(n); // (offset + n) - offset
        double denominator = static_cast<double>(*(last - 1) - *first);

        slope_ = denominator != 0.0 ? numerator/denominator * compression_factor : 0.0;
        intercept_ = offset * compression_factor - slope_ * *first;
    }

    /**
     * Returns the estimated y-value of @p x.
     * @param x to estimate a y-value for
     * @return the estimated y-value for @p x
     */
    template<typename X>
    double predict(const X x) const { return std::fma(slope_, static_cast<double>(x), intercept_); }

    /**
     * Returns the slope of the linear segment.
     * @return the slope of the linear segment
     */
    double slope() const { return slope_; }

    /**
     * Returns the y-intercept of the linear segment.
     * return the y-intercept of the linear segment
     */
    double intercept() const { return intercept_; }

    /**
     * Returns the size of the linear segment in bytes.
     * @return segment size in bytes.
     */
    std::size_t size_in_bytes() { return 2 * sizeof(double); }

    /**
     * Writes the mathematical representation of the linear segment to an output stream.
     * @param out output stream to write the linear segment to
     * @param m the linear segment
     * @returns the output stream
     */
    friend std::ostream & operator<<(std::ostream &out, const LinearSpline &m) {
        return out << m.slope() << " * x + " << m.intercept();
    }
};


/**
 * A linear regression model that fits a straight line to minimize the mean squared error.
 *
 * We assume that x-values are sorted in ascending order and y-values are handed implicitly where @p offset and @p
 * offset + distance(first, last) are the first and last y-value, respectively. The y-values can be scaled by
 * providing a @p compression_factor.
 */
class LinearRegression
{
    private:
    double slope_;     ///< The slope of the linear function.
    double intercept_; ///< The y-intercept of the lienar function.

    public:
    /*
     * Default constructor.
     */
    LinearRegression() = default;

    /**
     * Builds a linaer regression model between on the given data points.
     * @param first, last iterators to the first and last x-value the linear regression is fit on
     * @param offset first y-value the linear regression is fit on
     * @param compression_factor by which the y-values are scaled
     */
    template<typename RandomIt>
    LinearRegression(RandomIt first, RandomIt last, std::size_t offset = 0, double compression_factor = 1.f) {
        std::size_t n = std::distance(first, last);

        if (n == 0) {
            slope_ = 0.f;
            intercept_ = 0.f;
            return;
        }
        if (n == 1) {
            slope_ = 0.f;
            intercept_ = static_cast<double>(offset) * compression_factor;
            return;
        }

        double mean_x = 0.0;
        double mean_y = 0.0;
        double c = 0.0;
        double m2 = 0.0;

        for (std::size_t i = 0; i != n; ++i) {
            auto x = *(first + i);
            std::size_t y = offset + i;

            double dx = x - mean_x;
            mean_x += dx /  (i + 1);
            mean_y += (y - mean_y) / (i + 1);
            c += dx * (y - mean_y);

            double dx2 = x - mean_x;
            m2 += dx * dx2;
        }

        double cov = c / (n - 1);
        double var = m2 / (n - 1);

        if (var == 0.f) {
            slope_  = 0.f;
            intercept_ = mean_y;
            return;
        }

        slope_ = cov / var * compression_factor;
        intercept_ = mean_y * compression_factor - slope_ * mean_x;
    }

    /**
     * Returns the estimated y-value of @p x.
     * @param x to estimate a y-value for
     * @return the estimated y-value for @p x
     */
    template<typename X>
    double predict(const X x) const { return std::fma(slope_, static_cast<double>(x), intercept_); }

    /**
     * Returns the slope of the linear regression model.
     * @return the slope of the linear regression model
     */
    double slope() const { return slope_; }

    /**
     * Returns the y-intercept of the linear regression model.
     * return the y-intercept of the linear regression model
     */
    double intercept() const { return intercept_; }

    /**
     * Returns the size of the linear regression model in bytes.
     * @return model size in bytes.
     */
    std::size_t size_in_bytes() { return 2 * sizeof(double); }

    /**
     * Writes the mathematical representation of the linear regression model to an output stream.
     * @param out output stream to write the linear regression model to
     * @param m the linear regression model
     * @returns the output stream
     */
    friend std::ostream & operator<<(std::ostream &out, const LinearRegression &m) {
        return out << m.slope() << " * x + " << m.intercept();
    }
};


/**
 * A model that fits a cubic segment from the first first to the last data point.
 *
 * We assume that x-values are sorted in ascending order and y-values are handed implicitly where @p offset and @p
 * offset + distance(first, last) are the first and last y-value, respectively. The y-values can be scaled by
 * providing a @p compression_factor.
 */
class CubicSpline
{
    private:
    double a_; ///< The cubic coefficient.
    double b_; ///< The quadric coefficietn.
    double c_; ///< The linear coefficient.
    double d_; ///< The y-intercept.

    public:
    /**
     * Default constructor.
     */
    CubicSpline() = default;

    /**
     * Builds a cubic segment between the first and last data point.
     * @param first, last iterators to the first and last x-value the cubic segment is fit on
     * @param offset first y-value the cubic segment is fit on
     * @param compression_factor by which the y-values are scaled
     */
    template<typename RandomIt>
    CubicSpline(RandomIt first, RandomIt last, std::size_t offset = 0, double compression_factor = 1.f) {
        std::size_t n = std::distance(first, last);

        if (n == 0) {
            a_ = 0.f;
            b_ = 0.f;
            c_ = 1.f;
            d_ = 0.f;
            return;
        }
        if (n == 1 or *first == *(last - 1)) {
            a_ = 0.f;
            b_ = 0.f;
            c_ = 0.f;
            d_ = static_cast<double>(offset) * compression_factor;
            return;
        }

        double xmin = static_cast<double>(*first);
        double ymin = static_cast<double>(offset) * compression_factor;
        double xmax = static_cast<double>(*(last - 1));
        double ymax = static_cast<double>(offset + n - 1) * compression_factor;

        double x1 = 0.0;
        double y1 = 0.0;
        double x2 = 1.0;
        double y2 = 1.0;

        double sxn, syn = 0.0;
        for (std::size_t i = 0; i != n; ++i) {
            double x = static_cast<double>(*(first + i));
            double y = static_cast<double>(offset + i) * compression_factor;
            sxn = (x - xmin) / (xmax - xmin);
            if (sxn > 0.0) {
                syn = (y - ymin) / (ymax - ymin);
                break;
            }
        }
        double m1 = (syn - y1) / (sxn - x1);

        double sxp, syp = 0.0;
        for (std::size_t i = 0; i != n; ++i) {
            double x = static_cast<double>(*(first + i));
            double y = static_cast<double>(offset + i) * compression_factor;
            sxp = (x - xmin) / (xmax - xmin);
            if (sxp < 1.0) {
                syp = (y - ymin) / (ymax - ymin);
                break;
            }
        }
        double m2 = (y2 - syp) / (x2 - sxp);

        if (std::pow(m1, 2.0) + std::pow(m2, 2.0) > 9.0) {
            double tau = 3.0 / std::sqrt(std::pow(m1, 2.0) + std::pow(m2, 2.0));
            m1 *= tau;
            m2 *= tau;
        }

        a_ = (m1 + m2 - 2.0)
            / std::pow(xmax - xmin, 3.0);

        b_ = -(xmax * (2.0 * m1 + m2 - 3.0) + xmin * (m1 + 2.0 * m2 - 3.0))
            / std::pow(xmax - xmin, 3.0);

        c_ = (m1 * std::pow(xmax, 2.0) + m2 * std::pow(xmin, 2.0) + xmax * xmin * (2.0 * m1 + 2.0 * m2 - 6.0))
            / std::pow(xmax - xmin, 3.0);

        d_ = -xmin * (m1 * std::pow(xmax, 2.0) + xmax * xmin * (m2 - 3.0) + std::pow(xmin, 2.0))
            / std::pow(xmax - xmin, 3.0);

        a_ *= ymax - ymin;
        b_ *= ymax - ymin;
        c_ *= ymax - ymin;
        d_ *= ymax - ymin;
        d_ += ymin;

        // Check if linear spline performs better.
        // LinearSpline ls(first, last, offset, compression_factor);

        // double ls_error = 0.f;
        // double cs_error = 0.f;

        // for (std::size_t i = 0; i != n; ++i) {
        //     double y = (offset +i) * compression_factor;
        //     auto key = *(first + i);
        //     double ls_pred = ls.predict(key);
        //     double cs_pred = predict(key);
        //     ls_error += std::abs(ls_pred - y);
        //     cs_error += std::abs(cs_pred - y);
        // }

        // if (ls_error < cs_error) {
        //     a_ = 0;
        //     b_ = 0;
        //     c_ = ls.slope();
        //     d_ = ls.intercept();
        // }
    }

    /**
     * Returns the estimated y-value of @p x.
     * @param x to estimate a y-value for
     * @return the estimated y-value for @p x
     */
    template<typename X>
    double predict(const X x) const {
        double x_ = static_cast<double>(x);
        double v1 = std::fma(a_, x_, b_);
        double v2 = std::fma(v1, x_, c_);
        double v3 = std::fma(v2, x_, d_);
        return v3;
    }

    /** Returns the cubic coefficient.
     * @return the cubic coefficient
     */
    double a() const { return a_; }

    /** Returns the quadric coefficient.
     * @return the quadric coefficient
     */
    double b() const { return b_; }

    /** Returns the linear coefficient.
     * @return the linear coefficient
     */
    double c() const { return c_; }

    /** Returns the y-intercept.
     * @return the y-intercept
     */
    double d() const { return d_; }

    /**
     * Returns the size of the cubic segment in bytes.
     * @return segment size in bytes.
     */
    std::size_t size_in_bytes() { return 4 * sizeof(double); }

    /**
     * Writes the mathematical representation of the cubic segment to an output stream.
     * @param out output stream to write the cubic segment to
     * @param m the cubic segment
     * @returns the output stream
     */
    friend std::ostream & operator<<(std::ostream &out, const CubicSpline &m) {
        return out << m.a() << " * x^3 + "
                   << m.b() << " * x^2 + "
                   << m.c() << " * x + d";
    }
};


/**
 * A radix model that projects a x-values to their most significant bits after eliminating the common prefix.
 *
 * We assume that x-values are sorted in ascending order and y-values are handed implicitly where @p offset and @p
 * offset + distance(first, last) are the first and last y-value, respectively. The y-values can be scaled by
 * providing a @p compression_factor.
 *
 * @tparam the type of x-values.
 */
template<typename X = uint64_t>
class Radix
{
    using x_type = X;

    private:
    x_type mask_; ///< The mask for parallel bits extract.

    public:
    /*
     * Default constructor.
     */
    Radix() = default;

    /**
     * Builds a radix model on the given data points.
     * @param first, last iterators to the first and last x-value the linear regression is fit on
     * @param offset first y-value the linear regression is fit on
     * @param compression_factor by which the y-values are scaled
     */
    template<typename RandomIt>
    Radix(RandomIt first, RandomIt last, std::size_t offset = 0, double compression_factor = 1.f) {
        std::size_t n = std::distance(first, last);

        if (n == 0) {
            mask_ = 0;
            return;
        }

        auto prefix = common_prefix_width(*first, *(last - 1)); // compute common prefix length

        if (prefix == (sizeof(x_type) * 8)) {
            mask_ = 42; // TODO: What should the mask be in this case?
            return;
        }

        // Determine radix width.
        std::size_t max = static_cast<std::size_t>(offset + n - 1) * compression_factor;
        bool is_mersenne = (max & (max + 1)) == 0; // check if max is 2^n-1
        auto radix = is_mersenne ? bit_width<std::size_t>(max) : bit_width<std::size_t>(max) - 1;

        // Mask all bits but the radix
        mask_ = (~(x_type)0 >> prefix) & (~(x_type)0 << ((sizeof(x_type) * 8) - radix - prefix)); //0xffff << prefix_
    }

    /**
     * Returns the estimated y-value of @p x.
     * @param x to estimate a y-value for
     * @return the estimated y-value for @p x
     */
    // double predict(const x_type x) const { return (x << prefix_) >> ((sizeof(x_type) * 8) - radix_); }
    double predict(const x_type x) const {
        if constexpr(sizeof(x_type) <= sizeof(unsigned)) {
            return _pext_u32(x, mask_);
        } else if constexpr(sizeof(x_type) <= sizeof(unsigned long long)) {
            return _pext_u64(x, mask_);
        } else {
            static_assert(sizeof(x_type) > sizeof(unsigned long long), "unsupported width of integral type");
        }
    }

    /**
     * Returns the mask used for parallel bits extraction.
     * @return the mask
     */
    uint8_t mask() const { return mask_; }

    /**
     * Returns the size of the radix model in bytes.
     * @return radix model size in bytes.
     */
    std::size_t size_in_bytes() { return sizeof(mask_); }

    /**
     * Writes a human readable representation of the radix model to an output stream.
     * @param out output stream to write the radix model to
     * @param m the radix model
     * @returns the output stream
     */
    friend std::ostream & operator<<(std::ostream &out, const Radix &m) {
        return out << "_pext(x, " << m.mask() << ")";
    }
};

} // namespace rmi
