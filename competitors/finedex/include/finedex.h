#ifndef __FINEDEX_H__
#define __FINEDEX_H__

#include "util.h"
#include "lr_model.h"
#include "lr_model_impl.h"
#include "aidel_model.h"
#include "aidel_model_impl.h"
#include "piecewise_linear_model.h"

namespace aidel {


template<class key_t, class val_t>
class FINEdex{
public:
    typedef aidel::AidelModel<key_t, val_t> aidelmodel_type;
    typedef LinearRegressionModel<key_t> lrmodel_type;
    typedef typename OptimalPiecewiseLinearModel<key_t, size_t>::CanonicalSegment canonical_segment;


public:
    inline FINEdex();
    inline FINEdex(int _maxErr, int _learning_step, float _learning_rate);
    ~FINEdex();
    void train(const std::vector<key_t> &keys, const std::vector<val_t> &vals, size_t _maxErr);
    void train_opt(const std::vector<key_t> &keys, const std::vector<val_t> &vals, size_t _maxErr);
    void print_models();
    void self_check();
    
    
    inline result_t find(const key_t &key, val_t &val);
    inline result_t insert(const key_t &key, const val_t &val);
    inline result_t update(const key_t &key, const val_t &val);
    inline result_t remove(const key_t &key);
    int scan(const key_t &key, const size_t n, std::vector<std::pair<key_t, val_t>> &result);
    size_t model_size();


private:
    void append_model(lrmodel_type &model, const typename std::vector<key_t>::const_iterator &keys_begin, 
                      const typename std::vector<val_t>::const_iterator &vals_begin, 
                      size_t size, int err);
    aidelmodel_type* find_model(const key_t &key);
    int locate_in_levelbin(key_t key, int model_pos);
    

private:
    std::vector<key_t> model_keys;
    std::vector<aidelmodel_type> aimodels;
    //root_type* root = nullptr;
    std::vector<canonical_segment> segments;

    int maxErr = 64;
    int learning_step = 1000;
    float learning_rate = 0.1;

};

}   // namespace aidel


#endif