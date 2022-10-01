#ifndef __LEARNED_INDEX_IMPL__
#define __LEARNED_INDEX_IMPL__

#include "../include/util.h"
#include "learned_index.h"
#include "../include/lr_model.h"


template<class key_t, class val_t>
inline LearnedIndex<key_t, val_t>::LearnedIndex() 
    : maxErr(64) 
{}

template<class key_t, class val_t>
inline LearnedIndex<key_t, val_t>::LearnedIndex(int _maxErr)
    : maxErr(_maxErr)
{}

template<class key_t, class val_t>
LearnedIndex<key_t, val_t>::~LearnedIndex()
{
    std::cout << "delete learned index" << std::endl;
    free(keys);
    free(vals);
    keys = nullptr;
    vals = nullptr;
    delete[] rmi_2nd_stage;
}


// ==============================  train ============================
template<class key_t, class val_t>
void LearnedIndex<key_t, val_t>::train(const std::vector<key_t> &keys_begin, 
                                           const std::vector<val_t> &vals_begin, 
                                           size_t _rmi_2nd_stage_model_n, int _maxErr)
{
    this->maxErr = _maxErr;
    train_rmi(keys_begin, vals_begin, _rmi_2nd_stage_model_n);
    // the number of invalid model?
    size_t invalid_number = 0;
    for(int model_i=0; model_i<rmi_2nd_stage_model_n; model_i++){
        if (rmi_2nd_stage[model_i].get_maxErr() > maxErr){
			invalid_number++;
		}
    }
    std::cout<< "learned index -> invalid mdoel: " << invalid_number << std::endl;
}


template<class key_t, class val_t>
void LearnedIndex<key_t, val_t>::train_rmi(const std::vector<key_t> &keys_begin, 
                                           const std::vector<val_t> &vals_begin, 
                                           size_t _rmi_2nd_stage_model_n)
{
    this->rmi_2nd_stage_model_n = _rmi_2nd_stage_model_n;
    COUT_THIS("Training learned index -> maxError="<<maxErr<<
              "\n\t\t-> the number of the 2nd stage: " << rmi_2nd_stage_model_n);
    
    delete[] rmi_2nd_stage;
    rmi_2nd_stage = new lrmodel_type[rmi_2nd_stage_model_n]();

    // store the keys
    size_t size = keys_begin.size();
    capacity = size;
    keys = (key_t *)malloc(sizeof(key_t)*size);
    vals = (val_t *)malloc(sizeof(val_t)*size);
    for(int i=0; i<size; i++){
        keys[i] = keys_begin[i];
        vals[i] = vals_begin[i];
    }
    // sorted?
    for(int i=1; i<size; i++){
        assert(keys[i]>keys[i-1]);
    }
    // train 1st stage
    rmi_1st_stage.train(keys_begin.begin(), size);
    this->rmi_1st_max_predict = rmi_1st_stage.predict(keys[size-1]);
    //this->rmi_1st_max_predict = keys_begin.size();
    //train 2nd stage
    std::vector<std::vector<key_t>> keys_dispatched(rmi_2nd_stage_model_n);
    std::vector<std::vector<size_t>> positions_dispatched(rmi_2nd_stage_model_n);
    for (size_t key_i = 0; key_i < size; ++key_i) {
        size_t model_i_pred = rmi_1st_stage.predict(keys[key_i]);
        size_t next_stage_model_i = pick_next_stage_model(model_i_pred);
        keys_dispatched[next_stage_model_i].push_back(keys[key_i]);
        positions_dispatched[next_stage_model_i].push_back(key_i);
    }
    for (size_t model_i = 0; model_i < rmi_2nd_stage_model_n; ++model_i) {
        std::vector<key_t> &keys = keys_dispatched[model_i];
        std::vector<size_t> &positions = positions_dispatched[model_i];
        rmi_2nd_stage[model_i].train(keys, positions);
        
        if(model_i%10000 == 0){
            fprintf(stderr, "\rTrained models: %2zd", model_i);
        }
    }
    fprintf(stderr,"\rTrained models: %2zd\n", rmi_2nd_stage_model_n);
}


template<class key_t, class val_t>
size_t LearnedIndex<key_t, val_t>::pick_next_stage_model(size_t pos_pred)
{
    size_t second_stage_model_i;
    assert(rmi_1st_max_predict > 0);
    second_stage_model_i = pos_pred * rmi_2nd_stage_model_n / rmi_1st_max_predict;

    if (second_stage_model_i >= rmi_2nd_stage_model_n) {
        second_stage_model_i = rmi_2nd_stage_model_n - 1;
    }

    return second_stage_model_i;
}



template<class key_t, class val_t>
size_t LearnedIndex<key_t, val_t>::predict(const key_t &key)
{
    size_t pos_pred = rmi_1st_stage.predict(key);
    size_t next_stage_model_i = pick_next_stage_model(pos_pred);
    return rmi_2nd_stage[next_stage_model_i].predict(key);
}

template<class key_t, class val_t>
std::vector<std::pair<size_t, size_t>> LearnedIndex<key_t, val_t>::predict_error(const std::vector<key_t> &keys)
{
    using pair_type = typename std::pair<size_t, size_t>;
    std::vector<pair_type> predicts;
    predicts.reserve(keys.size());

    for (auto i = 0; i < keys.size(); i++) {
        key_t key = keys[i];
        size_t pos_pred = rmi_1st_stage.predict(key);
        size_t next_stage_model_i = pick_next_stage_model(pos_pred);
        size_t pos = rmi_2nd_stage[next_stage_model_i].predict(key);
        pos = pos >= capacity ? (capacity - 1) : pos;

        size_t e;
        if(i>pos) e = i-pos;
        else e=pos-i;
        predicts.push_back(pair_type(pos, e));
        // assert(e <= maxErr + 1);
    }
    return predicts;
}




template <class key_t, class val_t>
size_t LearnedIndex<key_t, val_t>::exponential_search_key(const key_t &key, size_t pos) const 
{
  if (capacity == 0) return 0;
  pos = (pos >= capacity ? (capacity - 1) : pos);

  int begin_i = 0, end_i = capacity;
  size_t step = 1;

  if (keys[pos] <= key) {
    begin_i = pos;
    end_i = begin_i + step;
    while (end_i < (int)capacity && keys[end_i] <= key) {
      step *= 2;
      begin_i = end_i;
      end_i = begin_i + step;
    }
    if (end_i >= (int)capacity) {
      end_i = capacity - 1;
    }
  } else {
    end_i = pos;
    begin_i = end_i - step;
    while (begin_i >= 0 && keys[begin_i] > key) {
      step *= 2;
      end_i = begin_i;
      begin_i = end_i - step;
    }
    if (begin_i < 0) {
      begin_i = 0;
    }
  }

  assert(begin_i >= 0);
  assert(end_i < (int)capacity);
  assert(begin_i <= end_i);

  // the real range is [begin_i, end_i], both inclusive.
  // we add 1 to end_i in order to find the insert position when the given key
  // is not exist
  end_i++;
  // find the largest position whose key equal to the given key
  while (end_i > begin_i) {
    // here the +1 term is used to avoid the infinte loop
    // where (end_i = begin_i + 1 && mid = begin_i && keys[mid] <= key)
    int mid = (begin_i + end_i) >> 1;
    if (keys[mid] < key) {
      begin_i = mid + 1;
    } else {
      // we should assign end_i with mid (not mid+1) in case infinte loop
      end_i = mid;
    }
  }

  assert(end_i == begin_i);
  assert(keys[end_i] == key || end_i == 0 || end_i == (int)capacity ||
         (keys[end_i - 1] < key && keys[end_i] > key));

  return end_i;
}

// ===================== search ======================
template<class key_t, class val_t>
inline bool LearnedIndex<key_t, val_t>::find(const key_t &key, val_t &val)
{
    size_t index_pos = predict(key);
    index_pos = exponential_search_key(key, index_pos);
    if(keys[index_pos]==key){
        val = vals[index_pos];
        return true;
    }
    return buffer.get(key, val);
}

// ====================== insert with buffer =================
template <class key_t, class val_t>
inline void LearnedIndex<key_t, val_t>::insert(const key_t &key, const val_t &val)
{
    size_t index_pos = predict(key);
    index_pos = exponential_search_key(key, index_pos);
    if(keys[index_pos]==key){
        return;
    }

    buffer.insert(key, val);
}


template <class key_t, class val_t>
inline void LearnedIndex<key_t, val_t>::insert_with_buffer(const key_t &key, const val_t &val)
{
    size_t pos_pred = rmi_1st_stage.predict(key);
    size_t next_stage_model_i = pick_next_stage_model(pos_pred);
    if (rmi_2nd_stage[next_stage_model_i].get_maxErr() <= maxErr) {
        size_t index_pos = rmi_2nd_stage[next_stage_model_i].predict(key);
        index_pos = exponential_search_key(key, index_pos);
        if(keys[index_pos]==key){
            return;
        }
    }
    buffer.insert(key, val);
}

// ====================== insert with buffer =================
template <class key_t, class val_t>
inline bool LearnedIndex<key_t, val_t>::remove(const key_t &key)
{
    size_t index_pos = predict(key);
    index_pos = exponential_search_key(key, index_pos);
    if(keys[index_pos]==key){
        return true;
    }

    return buffer.remove(key);
}

template<class key_t, class val_t>
inline bool LearnedIndex<key_t, val_t>::find_with_buffer(const key_t &key, val_t &val)
{
    size_t pos_pred = rmi_1st_stage.predict(key);
    size_t next_stage_model_i = pick_next_stage_model(pos_pred);
    if (rmi_2nd_stage[next_stage_model_i].get_maxErr() <= maxErr) {
        size_t index_pos = rmi_2nd_stage[next_stage_model_i].predict(key);
        index_pos = exponential_search_key(key, index_pos);
        if(keys[index_pos]==key){
            val = vals[index_pos];
            return true;
        }
    }

    return buffer.get(key, val);
}

// ==================== update ======================== 
template<class key_t, class val_t>
inline bool LearnedIndex<key_t, val_t>::update(const key_t &key, const val_t &val)
{
    size_t index_pos = predict(key);
    index_pos = exponential_search_key(key, index_pos);
    if(keys[index_pos]==key){
        vals[index_pos] = val;
        return true;
    }

    return buffer.update(key, val);
}

// ==================== update ======================== 
template<class key_t, class val_t>
int LearnedIndex<key_t, val_t>::scan(const key_t &key, const int n, std::vector<std::pair<key_t, val_t>> &result)
{
    result.clear();
    size_t index_pos = predict(key);
    index_pos = exponential_search_key(key, index_pos);

    std::vector<std::pair<key_t, val_t>> result_in_tree;
    buffer.scan(key, n, result_in_tree);
    size_t i = 0;
    size_t remaining = n;
    while(remaining>0) {
        if(i<result_in_tree.size() && index_pos<capacity) {
            if(keys[index_pos] < result_in_tree[i].first){
                result.push_back(result_in_tree[i]);
                i++;
            } else {
                result.push_back(std::pair<key_t, val_t>(keys[index_pos], vals[index_pos]));
                index_pos++;
            }
            remaining--;
        } else if(i<result_in_tree.size()) {
            assert(index_pos >= capacity);
            result.push_back(result_in_tree[i]);
            i++;
            remaining--;
        } else if(index_pos<capacity){
            assert(i >= result_in_tree.size());
            result.push_back(std::pair<key_t, val_t>(keys[index_pos], vals[index_pos]));
            index_pos++;
            remaining--;
        } else {
            break;
        }
    }
    return remaining;

}


template<class key_t, class val_t>
void LearnedIndex<key_t, val_t>::train_with_buffer(const std::vector<key_t> &keys_begin, 
                                           const std::vector<val_t> &vals_begin, 
                                           size_t _rmi_2nd_stage_model_n, int _maxErr)
{
    train(keys_begin, vals_begin, _rmi_2nd_stage_model_n, _maxErr);
    for (int i = 0; i < capacity; i++){
		key_t key = keys[i];
        size_t pos_pred = rmi_1st_stage.predict(key);
        size_t next_stage_model_i = pick_next_stage_model(pos_pred);
		
		if (rmi_2nd_stage[next_stage_model_i].get_maxErr() > maxErr){
			buffer.insert(key, vals[i]);
		}
	}
}


#endif