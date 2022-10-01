#pragma once


#include "level_bin.h"
#include "lr_model.h"
#include "util.h"
#include "plr.hpp"

namespace finedex{

template<class key_t, class val_t>
class FineModel {
public:
    typedef LinearRegressionModel<key_t> lrmodel_type;
    typedef LevelBin<key_t, val_t> levelbin_type;
    typedef FineModel<key_t, val_t> finemodel_type;
    typedef PLR<key_t, size_t> OptimalPLR;

    class Model_or_bin;
    using model_or_bin_t = class Model_or_bin;

private:
    lrmodel_type* model = nullptr;
    key_t* keys = nullptr;
    val_t* vals = nullptr;
    bool* valid_flag = nullptr;
    model_or_bin_t** mobs = nullptr;
    const size_t capacity;

    size_t Epsilon=32;

public:
    explicit FineModel(double slope, double intercept, size_t epsilon,
                       const typename std::vector<key_t>::const_iterator &keys_begin,
                       const typename std::vector<val_t>::const_iterator &vals_begin, 
                       size_t size) : capacity(size), Epsilon(epsilon)
    {
        model=new lrmodel_type(slope, intercept, epsilon);
        keys = (key_t *)malloc(sizeof(key_t)*size);
        vals = (val_t *)malloc(sizeof(val_t)*size);
        valid_flag = (bool*)malloc(sizeof(bool)*size);
        for(int i=0; i<size; i++){
            keys[i] = *(keys_begin+i);
            vals[i] = *(vals_begin+i);
            valid_flag[i] = true;
        }
        mobs = (model_or_bin_t**)malloc(sizeof(model_or_bin_t*)*(size+1));
        for(int i=0; i<size+1; i++){
            mobs[i]=nullptr;
        }
    }

    inline size_t get_capacity() {return capacity;}

    inline key_t get_lastkey() { return keys[capacity-1]; }

    inline key_t get_firstkey() { return keys[0]; }

    void print() {
        LOG(4)<<"[print finemodel] capacity:"<<capacity<<" -->";
        model->print();
        if(mobs[0]) mobs[0]->print();
        for(size_t i=0; i<capacity; i++){
            std::cout<<"keys["<<i<<"]: " <<keys[i] << std::endl;
            if(mobs[i+1]) {
                mobs[i+1]->print();
            }
        }
    }

    result_t find(const key_t &key, val_t &val)
    {
        size_t pos=0;
        if(find_array(key, pos)) {
            if(valid_flag[pos]){
                val=vals[pos];
                return result_t::ok;
            }
            return result_t::failed;
        }

        memory_fence();
        if(mobs[pos]==nullptr) return result_t::failed;
        return mobs[pos]->find(key, val);
    }

    bool find_array(const key_t &key, size_t &pos) {
        auto [pre, lo, hi] = this->model->predict(key, capacity);
        if(lo==hi) pos = lo;
        else pos = binary_search_branchless(keys+lo, hi-lo+1, key) + lo;
        if(pos>=capacity) {
            pos=capacity-1;
            return false;
        }
        if(keys[pos]==key) return true;
        pos = pos==capacity-1? ++pos:pos;
        return false; 
    }


    result_t find_debug(const key_t &key)
    {
        auto [pre, lo, hi] = this->model->predict(key, capacity);
        size_t pos;
        if(lo==hi) pos = lo;
        else pos = binary_search_branchless(keys+lo, hi-lo, key) + lo;
        LOG(5) <<"key: "<<key <<", [pre, lo, hi, pos]: "<<pre<<", "<<lo<<", "<<hi<<", "<<pos;
        return result_t::ok;
    }

    void self_check()
    {
        for(size_t i=1; i<capacity; i++){
            assert(keys[i]>keys[i-1]);
            val_t dummy_val=0;
            auto res = find(keys[i], dummy_val);
            if(res!=Result::ok) {
                auto [pre, lo, hi] = this->model->predict(keys[i], capacity);
                LOG(5)<<"[fineModel error] i: "<< i<< ", key: "<<keys[i]<<" , [pre, lo, hi]: "<<pre<<", "<<lo<<", "<<hi;  
                exit(0);         
            }
        }
        for(size_t i=0; i<=capacity; i++){
            if(mobs[i]) mobs[i]->self_check();
        }
    }

    // ======================= update =========================
    result_t update(const key_t &key, const val_t &val)
    {
        size_t pos=0;
        if(find_array(key, pos)) {
            if(valid_flag[pos]){
                vals[pos] = val;
                return result_t::ok;
            }
            return result_t::failed;
        }
        memory_fence();
        if(mobs[pos]==nullptr) return result_t::failed;
        return mobs[pos]->update(key, val);
    }

    // =============================== insert =======================
    result_t insert(const key_t &key, const val_t &val)
    {
        size_t pos=0;
        if(find_array(key, pos)) {
            if(valid_flag[pos]){
                return result_t::failed;
            } else {
                valid_flag[pos] = true;
                vals[pos] = val;
                return result_t::ok;
            }
        }
        //insert into model or bin
        //LOG(5)<<"insert key: "<<key<<" into bin: "<<pos;
        if(mobs[pos]==nullptr){
            model_or_bin_t* mob = new model_or_bin_t();
            memory_fence();
            if(!mobs[pos]) mobs[pos] = mob;
        }
        return mobs[pos]->insert(key, val, Epsilon);
    }

    // ========================== remove =====================
    result_t remove(const key_t &key)
    {
        size_t pos=0;
        if(find_array(key, pos)) {
            if(valid_flag[pos]){
                valid_flag[pos] = false;
                return result_t::ok;
            } 
            return result_t::failed;
        }
        memory_fence();
        if(mobs[pos]==nullptr) return result_t::failed;
        return mobs[pos]->remove(key);
    }

    // ========================== scan ===================
    int scan(const key_t &key, const size_t n, std::vector<std::pair<key_t, val_t>> &result)
    {
        size_t remaining = n;
        size_t pos = 0;
        find_array(key, pos);
        while(remaining>0 && pos<=capacity) {
            if(pos<capacity && valid_flag[pos] && keys[pos]>=key){
                result.push_back(std::pair<key_t, val_t>(keys[pos], vals[pos]));
                remaining--;
                if(remaining<=0) break;
            }
            memory_fence();
            if(mobs[pos]!=nullptr)
                remaining = mobs[pos]->scan(key, remaining, result);
            pos++;
        }
        return remaining;
    }

};

template<class key_t, class val_t>
class FineModel<key_t, val_t>::Model_or_bin
{
private:
    levelbin_type* lb;
    std::vector<finemodel_type> models;
    std::vector<key_t> model_keys;

    bool volatile isbin = true;   // true = lb, false = ai
    volatile uint8_t locked = 0;
    size_t Epsilon;

    void lock(){
        uint8_t unlocked = 0, locked = 1;
        while (unlikely(cmpxchgb((uint8_t *)&this->locked, unlocked, locked) != unlocked))
          ;
    }
    void unlock(){
        locked = 0;
    }

public:
    Model_or_bin() : lb(new levelbin_type()), isbin(true), models(), model_keys(), locked(0) {}

    void print()
    {
        if(isbin) lb->print(std::cout);
        else {
            for(int i=0; i<models.size(); i++) models[i].print();
        }
    }

    void self_check(){
        if(isbin) lb->self_check();
        else {
            for(int i=0; i<models.size(); i++) models[i].self_check();
        }
    }

    result_t find(const key_t &key, val_t &val) {
        lock();
        result_t res = result_t::failed;
        if(isbin){
            res = lb->find(key, val);
        } else{
            size_t model_pos = binary_search_branchless(&model_keys[0], model_keys.size(), key);
            if(model_pos >= models.size())
              model_pos = models.size()-1;
            res = models[model_pos].find(key, val);
        }
        assert(res!=result_t::retrain);
        unlock();
        return res;
    }

    result_t insert(const key_t &key, const val_t &val, size_t epsilon) {
        lock();
        result_t res = result_t::failed;
        if(isbin) {           // insert into bin
            res = lb->insert(key, val);
            if(res!=result_t::retrain) {
                unlock();
                return res;
            }

            //retrain
            // resort the data and train the model
            std::vector<key_t> retrain_keys;
            std::vector<val_t> retrain_vals;
            lb->resort(retrain_keys, retrain_vals);
            retrain(retrain_keys, retrain_vals, epsilon);
            
            memory_fence();
            delete lb;
            isbin = false;
        } 
        // insert into model
        size_t model_pos = binary_search_branchless(&model_keys[0], model_keys.size(), key);
        if(model_pos >= models.size())
          model_pos = models.size()-1;
        res = models[model_pos].insert(key, val);
        unlock();
        return res;
    }

    result_t update(const key_t &key, const val_t &val)
    {
        lock();
        result_t res = result_t::failed;
        if(isbin){
            res = lb->update(key, val);
        } else{
            size_t model_pos = binary_search_branchless(&model_keys[0], model_keys.size(), key);
            if(model_pos >= models.size())
              model_pos = models.size()-1;
            res = models[model_pos].update(key, val);
        }
        assert(res!=result_t::retrain);
        unlock();
        return res;
    }

    result_t remove(const key_t &key)
    {
        lock();
        result_t res = result_t::failed;
        if(isbin){
            res = lb->remove(key);
        } else{
            size_t model_pos = binary_search_branchless(&model_keys[0], model_keys.size(), key);
            if(model_pos >= models.size())
              model_pos = models.size()-1;
            res = models[model_pos].remove(key);
        }
        assert(res!=result_t::retrain);
        unlock();
        return res;
    }

    int scan(const key_t &key, const size_t n, std::vector<std::pair<key_t, val_t>> &result)
    {
        size_t remaining = n;
        lock();
        if(isbin){
            remaining = lb->scan(key, remaining, result);
        } else {
            size_t model_pos = binary_search_branchless(&model_keys[0], model_keys.size(), key);
            if(model_pos >= models.size())
              model_pos = models.size()-1;
            remaining = models[model_pos].scan(key, remaining, result);
        }
        unlock();
        return remaining;
    }

private:
    void retrain(const std::vector<key_t> &keys, 
               const std::vector<val_t> &vals, size_t epsilon)
    {
        assert(keys.size() == vals.size());
        if(keys.size()==0) return;
        this->Epsilon = epsilon;
        //LOG(2) << "ReTraining data: "<<keys.size()<<", Epsilon: "<<Epsilon;

        OptimalPLR* opt = new OptimalPLR(Epsilon-1);
        key_t p = keys[0];
        size_t pos=0;
        opt->add_point(p, pos);
        auto k_iter = keys.begin();
        auto v_iter = vals.begin();
        for(int i=1; i<keys.size(); i++) {
          key_t next_p = keys[i];
          if (next_p == p){
            LOG(5)<<"DUPLICATE keys";
            exit(0);
          }
          p = next_p;
          pos++;
          if(!opt->add_point(p, pos)) {
            auto cs = opt->get_segment();
            auto[cs_slope, cs_intercept] = cs.get_slope_intercept();
            append_model(cs_slope, cs_intercept, Epsilon, k_iter, v_iter, pos);
            k_iter += pos;
            v_iter += pos;
            pos=0;
            opt = new OptimalPLR(Epsilon-1);
            opt->add_point(p, pos);
          }
        }
        auto cs = opt->get_segment();
        auto[cs_slope, cs_intercept] = cs.get_slope_intercept();
        append_model(cs_slope, cs_intercept, Epsilon, k_iter, v_iter, ++pos);
        //LOG(2) << "ReTraining models: "<<models.size();
    }

    void append_model(double slope, double intercept, size_t epsilon,
                      const typename std::vector<key_t>::const_iterator &keys_begin,
                      const typename std::vector<val_t>::const_iterator &vals_begin, 
                      size_t size)
    {
        models.emplace_back(slope, intercept, epsilon, keys_begin, vals_begin, size);
        model_keys.push_back(models.back().get_lastkey());
    }
    
};


} //namespace findex


