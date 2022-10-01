#ifndef __LR_MODEL_IMPL_H__
#define __LR_MODEL_IMPL_H__

#include "lr_model.h"
#include "util.h"


template<class key_t>
inline LinearRegressionModel<key_t>::LinearRegressionModel(){}

template<class key_t>
inline LinearRegressionModel<key_t>::LinearRegressionModel(double w, double b)
{
    weights[0] = w;
    weights[1] = b;
}

template<class key_t>
LinearRegressionModel<key_t>::~LinearRegressionModel(){}


template<class key_t>
void LinearRegressionModel<key_t>::train(const typename std::vector<key_t>::const_iterator &it, size_t size)
{
    std::vector<key_t> trainkeys(size);
    std::vector<size_t> positions(size);
    for(size_t i=0; i<size; i++){
        trainkeys[i]=*(it+i);
        positions[i] = i;
    }
    train(trainkeys, positions);
}


template<class key_t>
void LinearRegressionModel<key_t>::train(const std::vector<key_t> &keys,
                                         const std::vector<size_t> &positions)
{
    assert(keys.size() == positions.size());
    if (keys.size() == 0) return;

    std::vector<double> model_keys(keys.size());
    std::vector<double *> key_ptrs(keys.size());
    for (size_t i = 0; i < keys.size(); i++) {
        model_keys[i] = keys[i];
    }

    if (positions.size() == 0) return;
    if (positions.size() == 1) {
        weights[0] = 0;
        weights[1] = positions[0];
        return;
    }
    // use multiple dimension LR when running tpc-c
    double x_expected = 0, y_expected = 0, xy_expected = 0,
           x_square_expected = 0;
    for (size_t key_i = 0; key_i < positions.size(); key_i++) {
        double key = model_keys[key_i];
        x_expected += key;
        y_expected += positions[key_i];
        x_square_expected += key * key;
        xy_expected += key * positions[key_i];
    }
    assert(x_expected>0);
    assert(y_expected>0);
    assert(x_square_expected>0);
    assert(xy_expected>0);
    
    x_expected /= positions.size();
    y_expected /= positions.size();
    x_square_expected /= positions.size();
    xy_expected /= positions.size();

    weights[0] = (xy_expected - x_expected * y_expected) /
                 (x_square_expected - x_expected * x_expected);
    weights[1] = (x_square_expected * y_expected - x_expected * xy_expected) /
                 (x_square_expected - x_expected * x_expected);
    maxErr = max_error(keys, positions);
}

template<class key_t>
void LinearRegressionModel<key_t>::print_weights() const {
    std::cout<< "Weight[0]: "<<weights[0] << " ,weight[1]: " <<weights[1] <<std::endl;
}

// ============ prediction ==============
template <class key_t>
size_t LinearRegressionModel<key_t>::predict(const key_t &key) const{
    double model_key = key;
    double res = weights[0] * model_key + weights[1];
    return res > 0 ? res : 0;
}

template <class key_t>
std::vector<size_t> LinearRegressionModel<key_t>::predict(const std::vector<key_t> &keys) const
{
    assert(keys.size()>0);
    std::vector<size_t> pred(keys.size());
    for(int i=0; i<keys.size(); i++)
    {
        pred[i] = predict(keys[i]);
    }
    return pred;
}

// =========== max__error ===========
template <class key_t>
size_t LinearRegressionModel<key_t>::max_error(
    const typename std::vector<key_t>::const_iterator &keys_begin,
    uint32_t size) {
    size_t max = 0;

    for (size_t key_i = 0; key_i < size; ++key_i) {
        long long int pos_actual = key_i;
        long long int pos_pred = predict(*(keys_begin + key_i));
        int error = std::abs(pos_actual - pos_pred);
        if (error > max) {
            max = error;
        }
    }
    return max;
}

template <class key_t>
size_t LinearRegressionModel<key_t>::max_error(const std::vector<key_t> &keys,
                                               const std::vector<size_t> &positions)
{
    size_t max = 0;

    for (size_t key_i = 0; key_i < keys.size(); ++key_i) {
        long long int pos_actual = positions[key_i];
        long long int pos_pred = predict(keys[key_i]);
        int error = std::abs(pos_actual - pos_pred);
        if (error > max) {
            max = error;
        }
    }
    return max;
}



#endif