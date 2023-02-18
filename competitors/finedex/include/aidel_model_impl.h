#ifndef __AIDEL_MODEL_IMPL_H__
#define __AIDEL_MODEL_IMPL_H__

#include "aidel_model.h"
#include "util.h"

namespace aidel{

template<class key_t, class val_t, class SearchClass>
AidelModel<key_t, val_t, SearchClass>::AidelModel(){
    model = nullptr;
    maxErr = 64;
    err = 0;
    keys = nullptr;
    levelbins = nullptr;
    capacity = 0;
}

template<class key_t, class val_t, class SearchClass>
AidelModel<key_t, val_t, SearchClass>::~AidelModel()
{
    if(model) model=nullptr;
    if(levelbins) levelbins=nullptr;
}

template<class key_t, class val_t, class SearchClass>
void AidelModel<key_t, val_t, SearchClass>::clear()
{
    if (levelbins){
        for (size_t i = 0; i < capacity + 1; ++ i){
            if (levelbins[i]){
                delete levelbins[i];
            }
            if (mobs[i]){
                if(mobs[i]->isbin){
                    delete mobs[i]->mob.lb;
                }else {
                    mobs[i]->mob.ai->clear();
                    delete mobs[i]->mob.ai;
                }
                delete mobs[i];
            }
        }
        free(mobs);
        free(levelbins);
    }
    if (model){
        free(keys);
        free(vals);
        free(valid_flag);
        delete model;
    }
}

template<class key_t, class val_t, class SearchClass>
AidelModel<key_t, val_t, SearchClass>::AidelModel(lrmodel_type &lrmodel, 
                                     const typename std::vector<key_t>::const_iterator &keys_begin, 
                                     const typename std::vector<val_t>::const_iterator &vals_begin, 
                                     size_t size, size_t _maxErr) : maxErr(_maxErr), capacity(size)
{
    model=new lrmodel_type(lrmodel.get_weight0(), lrmodel.get_weight1());
    keys = (key_t *)malloc(sizeof(key_t)*size);
    vals = (val_t *)malloc(sizeof(val_t)*size);
    valid_flag = (bool*)malloc(sizeof(bool)*size);
    for(int i=0; i<size; i++){
        keys[i] = *(keys_begin+i);
        vals[i] = *(vals_begin+i);
        valid_flag[i] = true;
    }
    levelbins = (levelbin_type**)malloc(sizeof(levelbin_type*)*(size+1));
    for(int i=0; i<size+1; i++){
        levelbins[i]=nullptr;
    }
    mobs = (model_or_bin_t**)malloc(sizeof(model_or_bin_t*)*(size+1));
    for(int i=0; i<size+1; i++){
        mobs[i]=nullptr;
    }
}

template<class key_t, class val_t, class SearchClass>
inline size_t AidelModel<key_t, val_t, SearchClass>::get_capacity()
{
    return capacity;
}

template<class key_t, class val_t, class SearchClass>
size_t AidelModel<key_t, val_t, SearchClass>::size() const{
    size_t size = sizeof(*this) + sizeof(lrmodel_type) + (sizeof(key_t) + sizeof(val_t) + sizeof(bool)) * capacity + (sizeof(levelbin_type*) + sizeof(model_or_bin_t*)) * (capacity + 1);
    for (size_t i = 0; i < capacity + 1; ++ i){
        if (levelbins[i]){
            size += levelbins[i]->size();
        }
        if (mobs[i]){
            if(mobs[i]->isbin){
                size += mobs[i]->mob.lb->size();
            }else {
                size += mobs[i]->mob.ai->size();
            }
        }
    }
    return size;
}

// =====================  print =====================
template<class key_t, class val_t, class SearchClass>
inline void AidelModel<key_t, val_t, SearchClass>::print_model()
{
    std::cout<<" capacity:"<<capacity<<" -->";
    model->print_weights();
    print_keys();
}

template<class key_t, class val_t, class SearchClass>
void AidelModel<key_t, val_t, SearchClass>::print_keys()
{
    if(levelbins[0]) levelbins[0]->print(std::cout);
    for(size_t i=0; i<capacity; i++){
        std::cout<<"keys["<<i<<"]: " <<keys[i] << std::endl;
        if(levelbins[i+1]) levelbins[i+1]->print(std::cout);
    }
}

template<class key_t, class val_t, class SearchClass>
void AidelModel<key_t, val_t, SearchClass>::print_model_retrain()
{
    std::cout<<"[print aimodel] capacity:"<<capacity<<" -->";
    model->print_weights();
    if(mobs[0]) {
        if(mobs[0]->isbin){
            mobs[0]->mob.lb->print(std::cout);
        }else {
            mobs[0]->mob.ai->print_model_retrain();
        }
    }
    for(size_t i=0; i<capacity; i++){
        std::cout<<"keys["<<i<<"]: " <<keys[i] << std::endl;
        if(mobs[i+1]) {
            if(mobs[i+1]->isbin){
                mobs[i+1]->mob.lb->print(std::cout);
            }else {
                mobs[i+1]->mob.ai->print_model_retrain();
            }
        }
    }
}

template<class key_t, class val_t, class SearchClass>
void AidelModel<key_t, val_t, SearchClass>::self_check()
{
    if(levelbins[0]) levelbins[0]->self_check();
    for(size_t i=1; i<capacity; i++){
        assert(keys[i]>keys[i-1]);
        if(levelbins[i]) levelbins[i]->self_check();
    }
    if(levelbins[capacity]) levelbins[capacity]->self_check();
}

template<class key_t, class val_t, class SearchClass>
void AidelModel<key_t, val_t, SearchClass>::self_check_retrain()
{
    for(size_t i=1; i<capacity; i++){
        assert(keys[i]>keys[i-1]);
    }
    for(size_t i=0; i<=capacity; i++){
        model_or_bin_t *mob = mobs[i];
        if(mob){
            if(mob->isbin){
                mob->mob.lb->self_check();
            } else {
                mob->mob.ai->self_check_retrain();
            }
        }
    }
}





// ============================ search =====================
template<class key_t, class val_t, class SearchClass>
bool AidelModel<key_t, val_t, SearchClass>::find(const key_t &key, val_t &val)
{
    size_t pos = predict(key);
    //pos = find_lower(key, pos);
    pos = locate_in_levelbin(key, pos);
    if(key == keys[pos]){
        if(valid_flag[pos]){
            val = vals[pos];
            return true;
        }
        return false;
    }
    int bin_pos = key<keys[pos]?pos:(pos+1);
    if(levelbins[bin_pos]==nullptr) return false; 
    typename levelbin_type::iterator it = levelbins[bin_pos]->find(key);
    if(it!=levelbins[bin_pos]->end()){
        val = it.data();
        return true;
    }
    return false;
}

template<class key_t, class val_t, class SearchClass>
bool AidelModel<key_t, val_t, SearchClass>::con_find(const key_t &key, val_t &val)
{
    size_t pos = predict(key);
    //pos = find_lower(key, pos);
    pos = locate_in_levelbin(key, pos);
    if(key == keys[pos]){
        if(valid_flag[pos]){
            val = vals[pos];
            return true;
        }
        return false;
    }
    int bin_pos = key<keys[pos]?pos:(pos+1);
    if(levelbins[bin_pos]==nullptr) return false;
    return levelbins[bin_pos]->con_find(key, val);
}

template<class key_t, class val_t, class SearchClass>
result_t AidelModel<key_t, val_t, SearchClass>::con_find_retrain(const key_t &key, val_t &val)
{
    size_t pos = predict(key);
    pos = locate_in_levelbin(key, pos);
    if(key == keys[pos]){
        if(valid_flag[pos]){
            val = vals[pos];
            return result_t::ok;
        }
        return result_t::failed;
    }
    int bin_pos = key<keys[pos]?pos:(pos+1);
    memory_fence();
    model_or_bin_t* mob = mobs[bin_pos];
    if(mob==nullptr) return result_t::failed;
    
    result_t res = result_t::failed;
    mob->lock();
    if(mob->isbin){
        res = mob->mob.lb->con_find_retrain(key, val);
        /*while(res == result_t::retrain) {
            while(mob->locked == 1)
                ;
            res = mob->mob.lb->con_find_retrain(key, val);
        }
        return res;*/
    } else{
        res = mob->mob.ai->con_find_retrain(key, val);
    }
    assert(res!=result_t::retrain);
    mob->unlock();
    return res;
}


template<class key_t, class val_t, class SearchClass>
inline size_t AidelModel<key_t, val_t, SearchClass>::predict(const key_t &key) {
    size_t index_pos = model->predict(key);
    return index_pos < capacity? index_pos:capacity-1;
}


/*template<class key_t, class val_t, class SearchClass>
inline size_t AidelModel<key_t, val_t, SearchClass>::find_lower(const key_t &key, const size_t pos){
    // predict
    //size_t index_pos = model->predict(key);
    //index_pos = index_pos < capacity? index_pos:capacity-1;
    size_t index_pos = pos;
    size_t avx_strid = 8;
    size_t upbound = capacity-avx_strid;
    size_t lowbound = avx_strid;

    // search
    if(key>=keys[index_pos]){
        bool flag = true;
        while(index_pos<=upbound){
            size_t offset = find_lower_avx(keys+index_pos, avx_strid, key);
            index_pos += offset;
            if(offset<8){
                flag = false;
                break;
            }
        }
        if(flag){
            index_pos += linear_search(keys+index_pos, capacity-index_pos, key);
        }
        return index_pos;
    } else {
        bool flag = true;
        while(index_pos>=lowbound){
            index_pos -= avx_strid;
            size_t offset = find_lower_avx(keys+index_pos, avx_strid, key);
            if(offset>0){
                flag = false;
                index_pos += offset;
                break;
            }
        }
        if(flag){
            index_pos = linear_search(keys, index_pos+1, key);
        }
        return index_pos;
    }
}

template<class key_t, class val_t, class SearchClass>
inline size_t AidelModel<key_t, val_t, SearchClass>::linear_search(const key_t *arr, int n, key_t key) {
    intptr_t i = 0;
    while (i < n) {
        if (arr[i] >= key)
            break;
        ++i;
    }
    return i;
}

template<class key_t, class val_t, class SearchClass>
inline size_t AidelModel<key_t, val_t, SearchClass>::find_lower_avx(const int *arr, int n, int key){
    __m256i vkey = _mm256_set1_epi32(key);
    __m256i cnt = _mm256_setzero_si256();
    for (int i = 0; i < n; i += 8) {
        __m256i mask0 = _mm256_cmpgt_epi32(vkey, _mm256_load_si256((__m256i *)&arr[i+0]));
        cnt = _mm256_sub_epi32(cnt, mask0);
    }
    __m128i xcnt = _mm_add_epi32(_mm256_extracti128_si256(cnt, 1), _mm256_castsi256_si128(cnt));
    xcnt = _mm_add_epi32(xcnt, _mm_shuffle_epi32(xcnt, SHUF(2, 3, 0, 1)));
    xcnt = _mm_add_epi32(xcnt, _mm_shuffle_epi32(xcnt, SHUF(1, 0, 3, 2)));

    return _mm_cvtsi128_si32(xcnt);
}

template<class key_t, class val_t, class SearchClass>
inline size_t AidelModel<key_t, val_t, SearchClass>::find_lower_avx(const int64_t *arr, int n, int64_t key) {
    __m256i vkey = _mm256_set1_epi64x(key);
    __m256i cnt = _mm256_setzero_si256();
    for (int i = 0; i < n; i += 8) {
      __m256i mask0 = _mm256_cmpgt_epi64(vkey, _mm256_load_si256((__m256i *)&arr[i+0]));
      __m256i mask1 = _mm256_cmpgt_epi64(vkey, _mm256_load_si256((__m256i *)&arr[i+4]));
      __m256i sum = _mm256_add_epi64(mask0, mask1);
      cnt = _mm256_sub_epi64(cnt, sum);
    }
    __m128i xcnt = _mm_add_epi64(_mm256_extracti128_si256(cnt, 1), _mm256_castsi256_si128(cnt));
    xcnt = _mm_add_epi64(xcnt, _mm_shuffle_epi32(xcnt, SHUF(2, 3, 0, 1)));
    return _mm_cvtsi128_si32(xcnt);
}*/

template<class key_t, class val_t, class SearchClass>
inline size_t AidelModel<key_t, val_t, SearchClass>::locate_in_levelbin(const key_t &key, const size_t pos)
{
    // predict
    //size_t index_pos = model->predict(key);
    size_t index_pos = pos;
    size_t upbound = capacity-1;
    //index_pos = index_pos <= upbound? index_pos:upbound;

    // search
    size_t begin, end;
    if(key > keys[index_pos]){
        begin = index_pos+1 < upbound? (index_pos+1):upbound;
        end = begin+maxErr < upbound? (begin+maxErr):upbound;
    } else {
        end = index_pos;
        begin = end>maxErr? (end-maxErr):0;
    }
    begin = SearchClass::upper_bound(keys + begin, keys + end + 1, key, keys + index_pos + 1) - keys;
    return begin >= 1 ? begin - 1 : 0;
    
    // assert(begin<=end);
    // while(begin != end){
    //     mid = (end + begin+2) / 2;
    //     if(keys[mid]<=key) {
    //         begin = mid;
    //     } else
    //         end = mid-1;
    // }
    // return begin;
}




// ======================= update =========================
template<class key_t, class val_t, class SearchClass>
result_t AidelModel<key_t, val_t, SearchClass>::update(const key_t &key, const val_t &val)
{
    size_t pos = predict(key);
    pos = locate_in_levelbin(key, pos);
    if(key == keys[pos]){
        if(valid_flag[pos]){
            vals[pos] = val;
            return result_t::ok;
        }
        return result_t::failed;
    }
    int bin_pos = key<keys[pos]?pos:(pos+1);
    memory_fence();
    model_or_bin_t* mob = mobs[bin_pos];
    if(mob==nullptr) return result_t::failed;
    
    result_t res = result_t::failed;
    mob->lock();
    if(mob->isbin){
        res = mob->mob.lb->update(key, val);
        /*while(res == result_t::retrain) {
            while(mob->locked == 1)
                ;
            res = mob->mob.lb->update(key, val);
        }
        return res;*/
    } else{
        res = mob->mob.ai->update(key, val);
    }
    assert(res!=result_t::retrain);
    mob->unlock();
    return res;
}




// =============================== insert =======================
template<class key_t, class val_t, class SearchClass>
inline bool AidelModel<key_t, val_t, SearchClass>::con_insert(const key_t &key, const val_t &val)
{
    size_t pos = predict(key);
    pos = locate_in_levelbin(key, pos);

    if(key == keys[pos]){
        if(valid_flag[pos]){
            return false;
        } else{
            valid_flag[pos] = true;
            vals[pos] = val;
            return true;
        }
    }
    int bin_pos = pos;
    bin_pos = key<keys[bin_pos]?bin_pos:(bin_pos+1);
    if(!levelbins[bin_pos]) {
        levelbin_type *lb = new levelbin_type();
        memory_fence();
        if(levelbins[bin_pos]) {
            delete lb;
            return levelbins[bin_pos]->con_insert(key, val);
        }
        levelbins[bin_pos] = lb;
    }
    assert(levelbins[bin_pos]!=nullptr);   
    return levelbins[bin_pos]->con_insert(key, val);
}

template<class key_t, class val_t, class SearchClass>
result_t AidelModel<key_t, val_t, SearchClass>::con_insert_retrain(const key_t &key, const val_t &val)
{
    size_t pos = predict(key);
    pos = locate_in_levelbin(key, pos);

    if(key == keys[pos]){
        if(valid_flag[pos]){
            return result_t::failed;
        } else{
            valid_flag[pos] = true;
            vals[pos] = val;
            return result_t::ok;
        }
    }
    int bin_pos = pos;
    bin_pos = key<keys[bin_pos]?bin_pos:(bin_pos+1);
    return insert_model_or_bin(key, val, bin_pos);
}


template<class key_t, class val_t, class SearchClass>
result_t AidelModel<key_t, val_t, SearchClass>::insert_model_or_bin(const key_t &key, const val_t &val, size_t bin_pos)
{
    // insert bin or model
    model_or_bin_t *mob = mobs[bin_pos];
    if(mob==nullptr){
        mob = new model_or_bin_t();
        mob->lock();
        mob->mob.lb = new levelbin_type();
        mob->isbin = true;
        memory_fence();
        if(mobs[bin_pos]){
            delete mob;
            return insert_model_or_bin(key, val, bin_pos);
        }
        mobs[bin_pos] = mob;
    } else{
        mob->lock();
    }
    assert(mob!=nullptr);
    assert(mob->locked == 1);
    result_t res = result_t::failed;
    if(mob->isbin) {           // insert into bin
        res = mob->mob.lb->con_insert_retrain(key, val);
        if(res == result_t::retrain){
            //COUT_THIS("[aimodel] Need Retrain: " << key);
            // resort the data and train the model
            std::vector<key_t> retrain_keys;
            std::vector<val_t> retrain_vals;
            mob->mob.lb->resort(retrain_keys, retrain_vals);
            lrmodel_type model;
            model.train(retrain_keys.begin(), retrain_keys.size());
            size_t err = model.get_maxErr();
            aidelmodel_type *ai = new aidelmodel_type(model, retrain_keys.begin(), retrain_vals.begin(), retrain_keys.size(), err);
            
            memory_fence();
            delete mob->mob.lb;
            mob->mob.ai = ai;
            mob->isbin = false;
            res = ai->con_insert_retrain(key, val);
            mob->unlock();
            return res;
        }
    } else{                   // insert into model
        res = mob->mob.ai->con_insert_retrain(key, val);
    }
    mob->unlock();
    return res;
}





// ========================== remove =====================
template<class key_t, class val_t, class SearchClass>
result_t AidelModel<key_t, val_t, SearchClass>::remove(const key_t &key)
{
    size_t pos = predict(key);
    pos = locate_in_levelbin(key, pos);
    if(key == keys[pos]){
        if(valid_flag[pos]){
            valid_flag[pos] = false;
            return result_t::ok;
        }
        return result_t::failed;
    }
    int bin_pos = key<keys[pos]?pos:(pos+1);
    return remove_model_or_bin(key, bin_pos);
}

template<class key_t, class val_t, class SearchClass>
result_t AidelModel<key_t, val_t, SearchClass>::remove_model_or_bin(const key_t &key, const int bin_pos)
{
    memory_fence();
    model_or_bin_t* mob = mobs[bin_pos];
    if(mob==nullptr) return result_t::failed;
    
    result_t res = result_t::failed;
    mob->lock();
    if(mob->isbin){
        res = mob->mob.lb->remove(key);
    } else{
        res = mob->mob.ai->remove(key);
    }
    assert(res!=result_t::retrain);
    mob->unlock();
    return res;
}


// ========================== scan ===================
template<class key_t, class val_t, class SearchClass>
int AidelModel<key_t, val_t, SearchClass>::scan(const key_t &key, const size_t n, std::vector<std::pair<key_t, val_t>> &result)
{
    size_t remaining = n;

    size_t pos = predict(key);
    pos = locate_in_levelbin(key, pos);
    while(remaining>0 && pos<=capacity) {
        if(pos<capacity && keys[pos]>=key){
            result.push_back(std::pair<key_t, val_t>(keys[pos], vals[pos]));
            remaining--;
            if(remaining<=0) break;
        }
        if(mobs[pos]!=nullptr){
            model_or_bin_t* mob = mobs[pos];
            if(mob->isbin){
                remaining = mob->mob.lb->scan(key, remaining, result);
            } else {
                remaining = mob->mob.ai->scan(key, remaining, result);
            }
        }
        pos++;
    }
    return remaining;
}

template<class key_t, class val_t, class SearchClass>
bool AidelModel<key_t, val_t, SearchClass>::range_scan(const key_t &lkey, const key_t &rkey, std::vector<std::pair<key_t, val_t>> &result){
    size_t pos = 0;
    if (lkey >= keys[0]){
        pos = predict(lkey);
        pos = locate_in_levelbin(lkey, pos);
    }
    bool is_end = false;
    if(keys[pos] <= lkey){
        if (keys[pos] == lkey){
            result.push_back(std::pair<key_t, val_t>(keys[pos], vals[pos]));
        }
        ++ pos;
    }
    while(!is_end && pos <= capacity) {
        memory_fence();
        model_or_bin_t* mob = mobs[pos];
        if(mob != nullptr){
            mob->lock();
            if(mob->isbin){
                is_end = mob->mob.lb->range_scan(lkey, rkey, result);
            } else {
                is_end = mob->mob.ai->range_scan(lkey, rkey, result);
            }
            mob->unlock();
        }
        if(!is_end && pos < capacity){
            if (keys[pos] > rkey){
                is_end = true;
                break;
            }
            result.push_back(std::pair<key_t, val_t>(keys[pos], vals[pos]));
        }
        ++ pos;
    }
    return is_end;
}

// ======================== resort data for retraining ===================
template<class key_t, class val_t, class SearchClass>
void AidelModel<key_t, val_t, SearchClass>::resort(std::vector<key_t> &keys, std::vector<val_t> &vals)
{
    typename levelbin_type::iterator it;
    for(size_t i=0; i<=capacity; i++){
        if(levelbins[i]){
            for(it = levelbins[i]->begin(); it!=levelbins[i]->end(); it++){
                keys.push_back(it.key());
                vals.push_back(it.data());
            }
        }
    }
}

} //namespace aidel



#endif