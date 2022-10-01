#ifndef __LEARNED_INDEX_H__
#define __LEARNED_INDEX_H__

#include "../include/util.h"
#include "../include/lr_model.h"
#include "../include/lr_model_impl.h"
#include "masstree.h"
#include "masstree_impl.h"

template<class key_t, class val_t>
class LearnedIndex{
    typedef LinearRegressionModel<key_t> lrmodel_type;
    //typedef stx::btree_map<key_t, val_t> btree_type;
    typedef FINEdex::Masstree<key_t, val_t> buffer_type;

public:
    inline LearnedIndex();
    inline LearnedIndex(int _maxErr);
    ~LearnedIndex();

    void train(const std::vector<key_t> &keys_begin, const std::vector<val_t> &vals_begin, 
                   size_t _rmi_2nd_stage_model_n, int _maxErr);
    void train_with_buffer(const std::vector<key_t> &keys_begin, const std::vector<val_t> &vals_begin, 
                   size_t _rmi_2nd_stage_model_n, int _maxErr);
    inline bool find(const key_t &key, val_t &val);
    inline void insert(const key_t &key, const val_t &val);
    inline void insert_with_buffer(const key_t &key, const val_t &val);
    inline bool find_with_buffer(const key_t &key, val_t &val);
    inline bool update(const key_t &key, const val_t &val);
    inline bool remove(const key_t &key);
    int scan(const key_t &key, const int n, std::vector<std::pair<key_t, val_t>> &result);
    std::vector<std::pair<size_t, size_t>> predict_error(const std::vector<key_t> &keys);

private:
    void train_rmi(const std::vector<key_t> &keys_begin, const std::vector<val_t> &vals_begin, 
                   size_t _rmi_2nd_stage_model_n);
    size_t pick_next_stage_model(size_t pos_pred);
    size_t predict(const key_t &key);
    size_t exponential_search_key(const key_t &key, size_t pos) const;

private:
    int maxErr=64;
    key_t* keys = nullptr;
    val_t* vals = nullptr;
    size_t capacity = 0;
    lrmodel_type rmi_1st_stage;
    lrmodel_type* rmi_2nd_stage = nullptr;
    size_t rmi_1st_max_predict = 0;
    size_t rmi_2nd_stage_model_n = 0;

    buffer_type buffer;
};



#endif