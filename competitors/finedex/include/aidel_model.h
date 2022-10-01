#ifndef __AIDEL_MODEL_H__
#define __AIDEL_MODEL_H__

#include "lr_model.h"
#include "lr_model_impl.h"
#include "level_bin_con.h"
#include "util.h"

namespace aidel{

template<class key_t, class val_t, class SearchClass>
class AidelModel {
public:
    typedef LinearRegressionModel<key_t> lrmodel_type;
    typedef aidel::LevelBinCon<key_t, val_t> levelbin_type;
    typedef aidel::AidelModel<key_t, val_t, SearchClass> aidelmodel_type;

    typedef struct model_or_bin {
        typedef union pointer{
            levelbin_type* lb;
            aidelmodel_type* ai;
        }pointer_t;
        pointer_t mob;
        bool volatile isbin = true;   // true = lb, false = ai
        volatile uint8_t locked = 0;

        void lock(){
            uint8_t unlocked = 0, locked = 1;
            while (unlikely(cmpxchgb((uint8_t *)&this->locked, unlocked, locked) !=
                            unlocked))
              ;
        }
        void unlock(){
            locked = 0;
        }
    }model_or_bin_t;

public:
    inline AidelModel();
    ~AidelModel();
    AidelModel(lrmodel_type &lrmodel, const typename std::vector<key_t>::const_iterator &keys_begin,
               const typename std::vector<val_t>::const_iterator &vals_begin, 
               size_t size, size_t _maxErr);
    inline size_t get_capacity();
    inline void print_model();
    void print_keys();
    void print_model_retrain();
    void self_check();
    void self_check_retrain();
    
    bool find(const key_t &key, val_t &val);
    bool con_find(const key_t &key, val_t &val);
    result_t con_find_retrain(const key_t &key, val_t &val);
    result_t update(const key_t &key, const val_t &val);
    inline bool con_insert(const key_t &key, const val_t &val);
    result_t con_insert_retrain(const key_t &key, const val_t &val);
    result_t remove(const key_t &key);
    int scan(const key_t &key, const size_t n, std::vector<std::pair<key_t, val_t>> &result);

    void resort(std::vector<key_t> &keys, std::vector<val_t> &vals);

    bool range_scan(const key_t &lkey, const key_t &rkey, std::vector<std::pair<key_t, val_t>> &result);
    size_t size() const;

    void clear();

private:
    inline size_t predict(const key_t &key);
    inline size_t locate_in_levelbin(const key_t &key, const size_t pos);
    inline size_t find_lower(const key_t &key, const size_t pos);
    inline size_t linear_search(const key_t *arr, int n, key_t key);
    inline size_t find_lower_avx(const int *arr, int n, int key);
    inline size_t find_lower_avx(const int64_t *arr, int n, int64_t key);
    result_t insert_model_or_bin(const key_t &key, const val_t &val, size_t bin_pos);
    result_t remove_model_or_bin(const key_t &key, const int bin_pos);


private:
    lrmodel_type* model = nullptr;
    size_t maxErr = 64;
    size_t err = 0;
    key_t* keys = nullptr;
    val_t* vals = nullptr;
    bool* valid_flag = nullptr;
    levelbin_type** levelbins = nullptr;
    model_or_bin_t** mobs = nullptr;

    const size_t capacity;

};

} //namespace aidel



#endif