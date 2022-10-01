#ifndef __AIDEL_IMPL_H__
#define __AIDEL_IMPL_H__

#include "finedex.h"
#include "util.h"
#include "aidel_model.h"
#include "aidel_model_impl.h"
#include "piecewise_linear_model.h"

namespace aidel {

template<class key_t, class val_t>
inline FINEdex<key_t, val_t>::FINEdex()
    : maxErr(64), learning_step(1000), learning_rate(0.1)
{
    //root = new root_type();
}

template<class key_t, class val_t>
inline FINEdex<key_t, val_t>::FINEdex(int _maxErr, int _learning_step, float _learning_rate)
    : maxErr(_maxErr), learning_step(_learning_step), learning_rate(_learning_rate)
{
    //root = new root_type();
}

template<class key_t, class val_t>
FINEdex<key_t, val_t>::~FINEdex(){
    //root = nullptr;
}

// ====================== train models ========================
template<class key_t, class val_t>
void FINEdex<key_t, val_t>::train(const std::vector<key_t> &keys, 
                                const std::vector<val_t> &vals, size_t _maxErr)
{
    assert(keys.size() == vals.size());
    maxErr = _maxErr;
    std::cout<<"training begin, length of training_data is:" << keys.size() <<" ,maxErr: "<< maxErr << std::endl;

    size_t last_n = keys.size();
    auto build_level = [&](auto epsilon, auto in_fun, auto out_fun) -> size_t {
        lrmodel_type* lr = nullptr;
        auto n_segments = make_segmentation_data(last_n, epsilon, in_fun, out_fun, lr);
        return n_segments;
    };

    // Build first level
    auto in_fun = [&](auto i) { return std::pair<key_t, size_t>(keys[i], i); };
    auto out_fun = [&](auto model, auto keys_begin, auto vals_begin, auto size, auto err) { 
        //aimodels.emplace_back(cs, nodetable);
        append_model(model, keys_begin, vals_begin, size, err);
    };
    last_n = build_level(_maxErr, in_fun, out_fun);

    //root = new root_type(model_keys);
    COUT_THIS("[aidle] get models -> "<< model_keys.size());
    assert(model_keys.size()==aimodels.size());
}


template<class key_t, class val_t>
void FINEdex<key_t, val_t>::append_model(lrmodel_type &model, 
                                       const typename std::vector<key_t>::const_iterator &keys_begin, 
                                       const typename std::vector<val_t>::const_iterator &vals_begin, 
                                       size_t size, int err)
{
    key_t key = *(keys_begin+size-1);
    
    // set learning_step
    int n = size/10;
    learning_step = 1;
    while(n!=0){
        n/=10;
        learning_step*=10;
    }
     
    assert(err<=maxErr);
    aidelmodel_type aimodel(model, keys_begin, vals_begin, size, maxErr);

    model_keys.push_back(key);
    aimodels.push_back(aimodel);
}

template<class key_t, class val_t>
typename FINEdex<key_t, val_t>::aidelmodel_type* FINEdex<key_t, val_t>::find_model(const key_t &key)
{
    // root 
    size_t model_pos = binary_search_branchless(&model_keys[0], model_keys.size(), key);
    if(model_pos >= aimodels.size())
        model_pos = aimodels.size()-1;
    return &aimodels[model_pos];
}


// ===================== print data =====================
template<class key_t, class val_t>
void FINEdex<key_t, val_t>::print_models()
{
    
    for(int i=0; i<model_keys.size(); i++){
        std::cout<<"model "<<i<<" ,key:"<<model_keys[i]<<" ->";
        aimodels[i].print_model();
    }

    
    
}

template<class key_t, class val_t>
void FINEdex<key_t, val_t>::self_check()
{
    for(int i=0; i<model_keys.size(); i++){
        aimodels[i].self_check();
    }
    
}


// =================== search the data =======================
template<class key_t, class val_t>
inline result_t FINEdex<key_t, val_t>::find(const key_t &key, val_t &val)
{   
    /*size_t model_pos = root->find(key);
    if(model_pos >= aimodels.size())
        model_pos = aimodels.size()-1;
    return aimodels[model_pos].con_find(key, val);*/

    return find_model(key)[0].con_find_retrain(key, val);

}


// =================  scan ====================
template<class key_t, class val_t>
int FINEdex<key_t, val_t>::scan(const key_t &key, const size_t n, std::vector<std::pair<key_t, val_t>> &result)
{
    size_t remaining = n;
    size_t model_pos = binary_search_branchless(&model_keys[0], model_keys.size(), key);
    if(model_pos >= aimodels.size())
        model_pos = aimodels.size()-1;
    while(remaining>0 && model_pos < aimodels.size()){
        remaining = aimodels[model_pos].scan(key, remaining, result);
    }
    return remaining;
}



// =================== insert the data =======================
template<class key_t, class val_t>
inline result_t FINEdex<key_t, val_t>::insert(
        const key_t& key, const val_t& val)
{
    return find_model(key)[0].con_insert_retrain(key, val);
    //return find_model(key)[0].con_insert(key, val);
}


// ================ update =================
template<class key_t, class val_t>
inline result_t FINEdex<key_t, val_t>::update(
        const key_t& key, const val_t& val)
{
    return find_model(key)[0].update(key, val);
    //return find_model(key)[0].con_insert(key, val);
}


// ==================== remove =====================
template<class key_t, class val_t>
inline result_t FINEdex<key_t, val_t>::remove(const key_t& key)
{
    return find_model(key)[0].remove(key);
    //return find_model(key)[0].con_insert(key, val);
}

// ========================== using OptimalLPR train the model ==========================
template<class key_t, class val_t>
void FINEdex<key_t, val_t>::train_opt(const std::vector<key_t> &keys, 
                                    const std::vector<val_t> &vals, size_t _maxErr)
{
    using pair_type = typename std::pair<size_t, size_t>;

    assert(keys.size() == vals.size());
    maxErr = _maxErr;
    std::cout<<"training begin, length of training_data is:" << keys.size() <<" ,maxErr: "<< maxErr << std::endl;

    segments = make_segmentation(keys.begin(), keys.end(), maxErr);
    COUT_THIS("[aidle] get models -> "<< segments.size());

    /*
    // ===== predict the positions ===========
    std::vector<pair_type> predicts;
    predicts.reserve(keys.size());

    auto it = segments.begin();
    auto [slope, intercept] = it->get_floating_point_segment(it->get_first_x());
    //COUT_THIS(slope<<", "<<intercept<<", "<<keys[0]<<", "<<it->get_first_x());

    for (auto i = 0; i < keys.size(); ++i) {
        if (i != 0 && keys[i] == keys[i - 1])
            continue;
        if (std::next(it) != segments.end() && std::next(it)->get_first_x() <= keys[i]) {
            ++it;
            std::tie(slope, intercept) = it->get_floating_point_segment(it->get_first_x());
        }

        auto pos = (keys[i] - it->get_first_x()) * slope + intercept;
        pos = pos<=0? 0:pos;
        size_t e;
        if(i>pos) e = i-pos;
        else e=pos-i;
        predicts.push_back(pair_type(pos, e));
        // assert(e <= maxErr + 1);
    }

    //assert(model_keys.size()==aimodels.size());
    return predicts;*/
}

template<class key_t, class val_t>
size_t FINEdex<key_t, val_t>::model_size(){
    return segments.size();
}



}    // namespace aidel


#endif