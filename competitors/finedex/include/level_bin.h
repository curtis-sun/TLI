#ifndef __LEVEL_BIN_CON_H__
#define __LEVEL_BIN_CON_H__

#include "util.h"

namespace aidel{

template<class _Key, class _Data, 
         class _Value = std::pair<_Key, _Data>,
         class _Compare = std::less<_Key>, 
         class _Alloc = std::allocator<_Value> >
class LevelBinCon{

public:
    //*** construct the types
    typedef _Key            key_type;
    typedef _Data           data_type;
    typedef _Value          value_type;
    typedef _Compare        key_compare;
    typedef _Alloc          allocator_type;
    typedef size_t          size_type;     //count the keys
    typedef std::pair<key_type, data_type>      pair_type;   // maybe different from value_type

    //*** construct the consts
    static const unsigned short rootslotmax = 8;
    static const unsigned short childslotmax = 16; 

private:
    struct bin {
        unsigned short slotuse;
        volatile uint8_t locked = 0;
        volatile uint64_t version = 0;

        inline void initialize(){
            slotuse = 0;
        }

        void lock(){
            uint8_t unlocked = 0, locked = 1;
            while (unlikely(cmpxchgb((uint8_t *)&this->locked, unlocked, locked) !=
                            unlocked))
              ;
        }
        void unlock(){
            locked = 0;
        }
    };
    struct root_bin : public bin {
        typedef typename _Alloc::template rebind<root_bin>::other alloc_type;
        key_type slotkey[rootslotmax];
        bin* children[rootslotmax+1];

        inline void initialize(){
            bin::initialize();
            if(sizeof(key_type)==sizeof(int32_t)){
                for(size_t i=0; i<rootslotmax; i++){
                    slotkey[i] = INT_MAX;
                }
            }
            if(sizeof(key_type)==sizeof(int64_t)){
                for(size_t i=0; i<rootslotmax; i++){
                    slotkey[i] = LONG_MAX;
                }
            }
        }
        inline bool isfull() const
        {
            return (bin::slotuse == rootslotmax);
        }
    };
    struct child_bin : public bin {
        typedef typename _Alloc::template rebind<child_bin>::other alloc_type;
        key_type slotkey[childslotmax];
        data_type slotdata[childslotmax];
        child_bin* prevbin;
        child_bin* nextbin;

        inline void initialize(){
            bin::initialize();
            prevbin = nextbin = NULL;
            if(sizeof(key_type)==sizeof(int32_t)){
                for(size_t i=0; i<childslotmax; i++){
                    slotkey[i] = INT_MAX;
                }
            }
            if(sizeof(key_type)==sizeof(int64_t)){
                for(size_t i=0; i<childslotmax; i++){
                    slotkey[i] = LONG_MAX;
                }
            }
        }
        inline bool isfull() const
        {
            return (bin::slotuse == childslotmax);
        }
    };

public:

    // ================== the iterator ===============
    class iterator {
    public:
        typedef typename LevelBinCon::key_type key_type;
        typedef iterator self;

    private:
        typename LevelBinCon::child_bin* curbin;
        unsigned short curslot;

    public:
        //*** Methods
        inline iterator()
          : curbin(NULL), curslot(0)
        {}

        inline iterator(typename LevelBinCon::child_bin *c, unsigned short s)
            : curbin(c), curslot(s)
        {}

        /// Key of the current slot
        inline const key_type& key() const
        {
            return curbin->slotkey[curslot];
        }

        /// Writable reference to the current data object
        inline data_type& data() const
        {
            return curbin->slotdata[curslot];
        }

        // ===================  the opeartors ================
        inline self& operator++(){
            if (curslot + 1 < curbin->slotuse) {
                ++curslot;
            }
            else if (curbin->nextbin != NULL) {
                curbin = curbin->nextbin;
                curslot = 0;
            }
            else {
                // this is end()
                curslot = curbin->slotuse;
            }

            return *this;
        }
        inline self operator++(int)
        {
            self tmp = *this;   // copy ourselves
            if (curslot + 1 < curbin->slotuse) {
                ++curslot;
            }
            else if (curbin->nextbin != NULL) {
                curbin = curbin->nextbin;
                curslot = 0;
            }
            else {
                // this is end()
                curslot = curbin->slotuse;
            }
            return tmp;
        }
        inline self& operator--()
        {
            if (curslot > 0) {
                --curslot;
            }
            else if (curbin->prevbin != NULL) {
                curbin = curbin->prevleaf;
                curslot = curbin->slotuse - 1;
            }
            else {
                // this is begin()
                curslot = 0;
            }
            return *this;
        }
        inline self operator--(int)
        {
            self tmp = *this;   // copy ourselves
            if (curslot > 0) {
                --curslot;
            }
            else if (curbin->prevbin != NULL) {
                curbin = curbin->prevleaf;
                curslot = curbin->slotuse - 1;
            }
            else {
                // this is begin()
                curslot = 0;
            }
            return tmp;
        }
        inline bool operator==(const self& x) const
        {
            return (x.curbin == curbin) && (x.curslot == curslot);
        }
        inline bool operator!=(const self& x) const
        {
            return (x.curbin != curbin) || (x.curslot != curslot);
        }
    };

public:
    //*** methods
    explicit inline LevelBinCon(const allocator_type &alloc = allocator_type()) 
        : m_root(NULL), m_headbin(NULL), m_tailbin(NULL), m_allocator(alloc) 
    {
        m_headbin = m_tailbin = allocate_childbin(); 
    }
    
    inline ~LevelBinCon()
    {
        clear();
    }

    // =================== the iterator ==================
    inline iterator begin()
    {
        return iterator(m_headbin, 0);
    }
    inline iterator end()
    {
        return iterator(m_tailbin, m_tailbin ? m_tailbin->slotuse : 0);
    }
    

private:
    // =========== allocat and deallocate the bins ==================
    typename root_bin::alloc_type root_bin_allocator(){
        return typename root_bin::alloc_type(m_allocator);
    }
    typename child_bin::alloc_type child_bin_allocator(){
        return typename child_bin::alloc_type(m_allocator);
    }
    inline root_bin* allocate_rootbin(){
        root_bin* n = new (root_bin_allocator().allocate(1)) root_bin();
        n->initialize();
        return n;
    }
    inline child_bin* allocate_childbin(){
        child_bin* n = new (child_bin_allocator().allocate(1)) child_bin();
        n->initialize();
        return n;
    }
    inline void free_rootbin(bin *n)
    {
        root_bin *ln = static_cast<root_bin*>(n);
        typename root_bin::alloc_type a(root_bin_allocator());
        a.destroy(ln);
        a.deallocate(ln, 1);
    }
    inline void free_childbin(bin *n)
    {
        child_bin *ln = static_cast<child_bin*>(n);
        typename child_bin::alloc_type a(child_bin_allocator());
        a.destroy(ln);
        a.deallocate(ln, 1);
    }
    void clear(){
        if(m_root){
            for (unsigned int slot = 0; slot < m_root->slotuse+1; ++slot)
            {
                // data objects are deleted by leaf_node's destructor
                free_childbin(m_root->children[slot]);
            }
            free_rootbin(m_root);
        } else if(m_headbin) {
            assert(m_headbin == m_tailbin);
            free_childbin(m_headbin);
        }
    }
    inline bool key_less(const key_type &a, const key_type b) const
    {
        return m_key_less(a, b);
    }
    inline bool key_lessequal(const key_type &a, const key_type b) const
    {
        return !m_key_less(b, a);
    }
    inline bool key_equal(const key_type &a, const key_type &b) const
    {
        return !m_key_less(a, b) && !m_key_less(b, a);
    }

public:
    //================== print the bins ====================
    void print(std::ostream &os) const
    {
        if (m_root) {
            const root_bin* r = m_root;
            os << "rootbin:" << "  " << r << std::endl;
            os <<"    ";
            for(unsigned int slot = 0; slot < r->slotuse; ++slot)
                os << " " << r->slotkey[slot];
            os << std::endl;
        }
        print_children(os);
    }
    void print_children(std::ostream &os) const
    {
        // size_t itemnum = 0;
        // const child_bin* c = m_headbin;
        // int child_id = 0;
        // while(c) {
        //     os << "--> childbin" << child_id++ << ":  " << c << std::endl;
        //     os <<" -> ";
        //     for(unsigned int slot = 0; slot < c->slotuse; ++slot)
        //         os << " " << c->slotkey[slot];
        //     os << std::endl;
        //     itemnum+=c->slotuse;
        //     c=c->nextbin;
        // }
        if(m_root){
            size_t itemnum = 0;
            const root_bin* r = m_root;
            child_bin* c;
            int child_id = 0;
            for(unsigned int slot=0; slot <= r->slotuse; ++slot) {
                os << "--> childbin" << child_id++ << ":  " << c << std::endl;
                os <<" -> ";
                c = static_cast<child_bin*>(r->children[slot]);
                itemnum+=c->slotuse;
                for(unsigned int cslot = 0; cslot < c->slotuse; ++cslot){
                    os << " " << c->slotkey[cslot];
                }
                os << std::endl;
            }
            os << "    total itemnum: " << itemnum << std::endl;
        } else {
            if(m_headbin){
                child_bin* c = m_headbin;
                assert(c->nextbin == nullptr);
                os << "--> childbin0 :  " << c << std::endl;
                os <<" -> ";
                for(unsigned int cslot = 0; cslot < c->slotuse; ++cslot){
                    os << " " << c->slotkey[cslot];
                }
                os << std::endl;
            }
        }         
    }

    // resort for retraining
    void resort(std::vector<key_type>& keys, std::vector<key_type>& vals) {
        assert(m_root);
        size_t itemnum = 0;
        const root_bin* r = m_root;
        child_bin* c;
        for(unsigned int slot=0; slot <= r->slotuse; ++slot) {
            c = static_cast<child_bin*>(r->children[slot]);
            itemnum+=c->slotuse;
            for(unsigned int cslot = 0; cslot < c->slotuse; ++cslot){
                keys.push_back(c->slotkey[cslot]);
                vals.push_back(c->slotdata[cslot]);
            }
        }
        assert(itemnum == keys.size());
        for(unsigned int i=1; i<keys.size(); i++){
            assert(keys[i]>keys[i-1]);
        }
    }

    // check  the correctness
    void self_check() const {
        if (m_root) {
            const root_bin* r = m_root;
            child_bin* c;
            for(unsigned int slot = 0; slot < r->slotuse; ++slot) {
                c = static_cast<child_bin*>(r->children[slot]);
                for(unsigned int cslot = 1; cslot < c->slotuse; ++cslot){
                    assert(c->slotkey[cslot] > c->slotkey[cslot-1]);
                }
                assert(r->slotkey[slot] == c->slotkey[c->slotuse-1]);
            }
            c = static_cast<child_bin*>(r->children[r->slotuse]);
            for(unsigned int cslot = 1; cslot < c->slotuse; ++cslot){
                assert(c->slotkey[cslot] > c->slotkey[cslot-1]);
            }
            assert(m_headbin == r->children[0] && m_tailbin == r->children[r->slotuse]);
        } else{
            if(m_headbin){
                child_bin* c = m_headbin;
                for(unsigned int cslot = 1; cslot < c->slotuse; ++cslot){
                    assert(c->slotkey[cslot] > c->slotkey[cslot-1]);
                }
                assert(m_headbin == c && m_tailbin == c);
            }
        }     
    }








    // ================== search in the bins =================
public:
    iterator find(const key_type &key)
    {
        memory_fence();
        if(m_root){
            root_bin* root = m_root;
            int slot = find_lower(root, key);
            child_bin* child = static_cast<child_bin*>(root->children[slot]);
            int cslot = find_lower(child, key);
            return (cslot < child->slotuse && key_equal(key, child->slotkey[cslot]))
            ? iterator(child, cslot) : end();
        } else{
            if(m_headbin){
                child_bin* child = m_headbin;
                //int cslot = find_lower_avx(child->slotkey, childslotmax, key);
                int cslot = find_lower(child, key);
                return (cslot < child->slotuse && key_equal(key, child->slotkey[cslot]))
                ? iterator(child, cslot) : end();
            } else {
                return end();
            }
        }
    }

    result_t con_find_retrain(const key_type &key, data_type &value) 
    {
        if(is_retraining) {
            return result_t::retrain;
        }

        uint64_t child_ver;
        child_bin* child = nullptr;
        result_t res = result_t::failed;
        memory_fence();
        if(m_root){
            root_bin* root = m_root;
            uint64_t root_ver = root->version;
            int slot = find_lower(root, key);
            memory_fence();
            bool locked = root->locked == 1;
            memory_fence();
            bool version_changed = root_ver != root->version;
            if (!locked && !version_changed) {
                child = static_cast<child_bin*>(root->children[slot]);
                child_ver = child->version;
            } else {
                return con_find_retrain(key, value);
            } 
        } else {
            if(m_headbin){
                child = m_headbin;
                child_ver = child->version;
            } else {
                return result_t::failed;
            }
        }
        while(true) {
            int cslot = find_lower(child, key);
            if(cslot < child->slotuse && key_equal(key, child->slotkey[cslot])){
                value = child->slotdata[cslot];
                res = result_t::ok;
            } else {
                res = result_t::failed;
            }
            memory_fence();
            bool locked = child->locked == 1;
            memory_fence();
            bool version_changed = child_ver != child->version;

            if (!locked && !version_changed) {
                return res;
            } else {
                // the node is changed, possibly split, so need to check its next
                child_ver = child->version;  // read version before reading next
                memory_fence();
                child_bin *next_bin = child->nextbin;  // in case this pointer changes
                while (next_bin && next_bin->slotkey[0] <= key) {
                    child = next_bin;
                    child_ver = child->version;
                    memory_fence();
                    next_bin = child->nextbin;
                }
            }
        }
    }

    bool con_find(const key_type &key, data_type &value)
    {
        uint64_t child_ver;
        child_bin* child = nullptr;
        bool result = false;
        memory_fence();
        if(m_root){
            root_bin* root = m_root;
            uint64_t root_ver = root->version;
            //int slot = find_lower_avx(root->slotkey, rootslotmax, key);
            int slot = find_lower(root, key);
            memory_fence();
            bool locked = root->locked == 1;
            memory_fence();
            bool version_changed = root_ver != root->version;
            if (!locked && !version_changed) {
                child = static_cast<child_bin*>(root->children[slot]);
                child_ver = child->version;
            } else {
                return con_find(key, value);
            }
        } else{
            if(m_headbin){
                child = m_headbin;
                child_ver = child->version;
            } else {
                return false;
            }
        }
        while(true) {
            //int cslot = find_lower_avx(child->slotkey, childslotmax, key);
            int cslot = find_lower(child, key);
            if(cslot < child->slotuse && key_equal(key, child->slotkey[cslot])){
                value = child->slotdata[cslot];
                result = true;
            } else {
                result = false;
            }
            memory_fence();
            bool locked = child->locked == 1;
            memory_fence();
            bool version_changed = child_ver != child->version;

            if (!locked && !version_changed) {
                return result;
            } else {
                // the node is changed, possibly split, so need to check its next
                child_ver = child->version;  // read version before reading next
                memory_fence();
                child_bin *next_bin = child->nextbin;  // in case this pointer changes
                while (next_bin && next_bin->slotkey[0] <= key) {
                    child = next_bin;
                    child_ver = child->version;
                    memory_fence();
                    next_bin = child->nextbin;
                }
            }
        }
    }

private:
    template <typename bin_type>
    inline int find_lower(const bin_type *n, const key_type& key) const
    {
        if (n->slotuse == 0) return 0;

        int lo = 0, hi = n->slotuse;
        while (lo < hi)
        {
            int mid = (lo + hi) >> 1;

            if (key_lessequal(key, n->slotkey[mid])) {
                hi = mid; // key <= mid
            }
            else {
                lo = mid + 1; // key > mid
            }
        }

        return lo;
    }

    inline size_t find_lower_avx(const int *arr, int n, int key){
        __m256i vkey = _mm256_set1_epi32(key);
        __m256i cnt = _mm256_setzero_si256();
        for (int i = 0; i < n; i += 8) {
            __m256i mask0 = _mm256_cmpgt_epi32(vkey, _mm256_loadu_si256((__m256i *)&arr[i+0]));
            cnt = _mm256_sub_epi32(cnt, mask0);
        }
        __m128i xcnt = _mm_add_epi32(_mm256_extracti128_si256(cnt, 1), _mm256_castsi256_si128(cnt));
        xcnt = _mm_add_epi32(xcnt, _mm_shuffle_epi32(xcnt, SHUF(2, 3, 0, 1)));
        xcnt = _mm_add_epi32(xcnt, _mm_shuffle_epi32(xcnt, SHUF(1, 0, 3, 2)));

        return _mm_cvtsi128_si32(xcnt);
    }
    inline size_t find_lower_avx(const int64_t *arr, int n, int64_t key) {
        __m256i vkey = _mm256_set1_epi64x(key);
        __m256i cnt = _mm256_setzero_si256();
        for (int i = 0; i < n; i += 8) {
          __m256i mask0 = _mm256_cmpgt_epi64(vkey, _mm256_loadu_si256((__m256i *)&arr[i+0]));
          __m256i mask1 = _mm256_cmpgt_epi64(vkey, _mm256_loadu_si256((__m256i *)&arr[i+4]));
          __m256i sum = _mm256_add_epi64(mask0, mask1);
          cnt = _mm256_sub_epi64(cnt, sum);
        }
        __m128i xcnt = _mm_add_epi64(_mm256_extracti128_si256(cnt, 1), _mm256_castsi256_si128(cnt));
        xcnt = _mm_add_epi64(xcnt, _mm_shuffle_epi32(xcnt, SHUF(2, 3, 0, 1)));
        return _mm_cvtsi128_si32(xcnt);
    }






    // ========================= update the bins ==============================
public:
    result_t update(const key_type &key, const data_type &val)
    {
        if(is_retraining) {
            return result_t::retrain;
        }

        uint64_t child_ver;
        child_bin* child = nullptr;
        result_t res = result_t::failed;
        memory_fence();
        if(m_root){
            root_bin* root = m_root;
            uint64_t root_ver = root->version;
            int slot = find_lower(root, key);
            memory_fence();
            bool locked = root->locked == 1;
            memory_fence();
            bool version_changed = root_ver != root->version;
            if (!locked && !version_changed) {
                child = static_cast<child_bin*>(root->children[slot]);
                child_ver = child->version;
            } else {
                return update(key, val);
            } 
        } else {
            if(m_headbin){
                child = m_headbin;
                child_ver = child->version;
            } else {
                return result_t::failed;
            }
        }
        while(true) {
            child->lock();
            if(child_ver != child->version) {   // child split?
                child->unlock();
                child_ver = child->version;
                memory_fence();
                child_bin *next_bin = child->nextbin;  // in case this pointer changes
                while (next_bin && next_bin->slotkey[0] <= key) {
                    child = next_bin;
                    child_ver = child->version;
                    memory_fence();
                    next_bin = child->nextbin;
                }
            } else {
                break;
            }
        }
        assert(child->locked == 1);
        int cslot = find_lower(child, key);
        if(cslot < child->slotuse && key_equal(key, child->slotkey[cslot])){
            child->slotdata[cslot] = val;

            memory_fence();
            child->version++;
            memory_fence();
            child->unlock();

            return result_t::ok;
        }
        child->unlock();
        return result_t::failed;
    } 
    





    // ================== insert into the bins ==========================
public:

    inline result_t con_insert_retrain(const key_type& key, const data_type& data)
    {   
        return con_insert_start_retrain(key, data);
    }

    bool con_insert(const key_type& key, const data_type& data)
    {   
        // result_t result = con_insert_start(key, data);
        // while(result == result_t::retry) {
        //     result = con_insert_start(key, data);
        // }
        // if(result==result_t::ok) return true;
        // return false;
        result_t result = con_insert_start(key, data);
        if(result==result_t::ok) return true;
        return false;
    }

private:
    result_t con_insert_start(const key_type& key, const data_type& value)
    {
        if(is_retraining){
            //COUT_THIS("level bin is retraining! "<<key);
            return result_t::retry;
        }
        memory_fence();
        if(m_root){
            //search the root
            root_bin* root = m_root;
            root->lock();
            if(is_retraining){
                COUT_THIS("level bin is retraining! "<<key);
                root->unlock();
                return result_t::retry;
            }
            uint64_t rootversion = root->version;
            //int slot = find_lower_avx(root->slotkey, rootslotmax, key);
            int slot = find_lower(root, key);
            // search curr bin
            child_bin* curr = static_cast<child_bin*>(root->children[slot]);
            curr->lock();
            //int curslot = find_lower_avx(curr->slotkey, childslotmax, key);
            int curslot = find_lower(curr, key);
            // equal?
            if (curslot < curr->slotuse && key_equal(key, curr->slotkey[curslot])) {
                curr->unlock();
                root->unlock();
                return result_t::failed;
            }
            // ===== insert ======
            // currnode has 
            if(!curr->isfull()){
                std::copy_backward(curr->slotkey + curslot, curr->slotkey + curr->slotuse,
                                   curr->slotkey + curr->slotuse+1);
                std::copy_backward(curr->slotdata + curslot, curr->slotdata + curr->slotuse,
                                   curr->slotdata + curr->slotuse+1);
                curr->slotkey[curslot] = key;
                curr->slotdata[curslot] = value;
                curr->slotuse++;
                // special case
                if ( curslot == curr->slotuse-1 && slot <= root->slotuse-1)
                {
                    // the insert is at the
                    // last slot of the old node. then the splitkey must be
                    // updated.
                    root->slotkey[slot] = key;
                }
                memory_fence();
                root->version++;
                curr->version++;
                memory_fence();
                curr->unlock();
                root->unlock();
                
                return result_t::ok;
            }
            // insert prevchild bin?
            if(slot > 0 && root->children[slot-1]->slotuse < childslotmax) {
                child_bin* prev = static_cast<child_bin*>(root->children[slot-1]);
                prev->lock();
                // insert the smallest key into prevchild
                if(curr->slotkey[0] > key){
                    prev->slotkey[prev->slotuse] = key;
                    prev->slotdata[prev->slotuse] = value;
                    prev->slotuse++;
                    root->slotkey[slot-1] = key;
                    memory_fence();
                    prev->version++;
                    root->version++;
                    memory_fence();
                    prev->unlock();
                    curr->unlock();
                    root->unlock();
                    return result_t::ok;
                } else {
                    prev->slotkey[prev->slotuse] = curr->slotkey[0];
                    prev->slotdata[prev->slotuse] = curr->slotdata[0];
                    prev->slotuse++;
                    root->slotkey[slot-1] = curr->slotkey[0];
                    
                    std::copy(curr->slotkey+1, curr->slotkey + curslot,
                              curr->slotkey);
                    std::copy(curr->slotdata + 1, curr->slotdata + curslot,
                              curr->slotdata);
                    curr->slotkey[curslot-1] = key;
                    curr->slotdata[curslot-1] = value;
                    // special case
                    if ( curslot == curr->slotuse && slot <= root->slotuse-1)
                    {
                        // the insert is at the
                        // last slot of the old node. then the splitkey must be
                        // updated.
                        root->slotkey[slot] = key;
                    }
                    memory_fence();
                    prev->version++;
                    curr->version++;
                    root->version++;
                    memory_fence();
                    prev->unlock();
                    curr->unlock();
                    root->unlock();
                    return result_t::ok;
                }
            } else {      //insert into current child and full
                // need retrain?
                if(root->isfull()){
                    std::cout<<"need retrain!" << key << std::endl;
                    is_retraining = true;
                    memory_fence();
                    curr->unlock();
                    root->unlock();
                    return result_t::retrain;
                }
                child_bin *newchild = allocate_childbin();
                newchild->lock();
                unsigned int mid = (curr->slotuse >> 1);
                // new child
                newchild->slotuse = curr->slotuse - mid;
                std::copy(curr->slotkey + mid, curr->slotkey + curr->slotuse,
                          newchild->slotkey);
                std::copy(curr->slotdata + mid, curr->slotdata + curr->slotuse,
                          newchild->slotdata);
                newchild->nextbin = curr->nextbin;
                newchild->prevbin = curr;
                // update root
                memory_fence();
                assert(m_root == root);
                assert(root->locked == 1); 
                key_type newkey = curr->slotkey[mid-1];
                std::copy_backward(root->slotkey + slot, root->slotkey + root->slotuse,
                                   root->slotkey + root->slotuse+1);
                std::copy_backward(root->children + slot, root->children + root->slotuse+1,
                                   root->children + root->slotuse+2);
                root->slotkey[slot] = newkey;
                root->children[slot + 1] = newchild;
                root->slotuse++;
                // update curr
                curr->slotuse = mid;
                curr->nextbin = newchild;
                if(newchild->nextbin == nullptr){
                    assert(curr==m_tailbin);
                    m_tailbin=newchild;
                }
                memory_fence(); 
                // insert 
                // check if insert slot is in the split sibling node
                // special case
                if(curslot == mid){
                    // special case: the node was split, and the insert is at the
                    // last slot of the old node. then the splitkey must be
                    // updated.
                    curr->slotkey[curslot] = key;
                    curr->slotdata[curslot] = value;
                    curr->slotuse++;
                    root->slotkey[slot] = key;
                } else if (curslot > mid) {
                    curslot -= curr->slotuse;
                    std::copy_backward(newchild->slotkey + curslot, newchild->slotkey + newchild->slotuse,
                                       newchild->slotkey + newchild->slotuse+1);
                    std::copy_backward(newchild->slotdata + curslot, newchild->slotdata + newchild->slotuse,
                                       newchild->slotdata + newchild->slotuse+1);
                    newchild->slotkey[curslot] = key;
                    newchild->slotdata[curslot] = value;
                    newchild->slotuse++;
                } else{
                    std::copy_backward(curr->slotkey + curslot, curr->slotkey + curr->slotuse,
                                       curr->slotkey + curr->slotuse+1);
                    std::copy_backward(curr->slotdata + curslot, curr->slotdata + curr->slotuse,
                                       curr->slotdata + curr->slotuse+1);
                    curr->slotkey[curslot] = key;
                    curr->slotdata[curslot] = value;
                    curr->slotuse++;
                }     
                memory_fence();
                root->version++;
                curr->version++;
                newchild->version++;
                memory_fence();
                curr->unlock();
                newchild->unlock();
                root->unlock();
                return result_t::ok;
            }
            
        } else {            // only occors when there is one bin
            uint64_t child_ver;
            child_bin* child = m_headbin;
            if(child == nullptr) {
                child = allocate_childbin();
                memory_fence();
                if(m_headbin) {
                    free_childbin(child);
                    return con_insert_start(key, value);
                    //return result_t::retry;
                }
                m_headbin = m_tailbin = child;
            }
            assert(child!=nullptr);
            child_ver = child->version;
            // lock
            int rslot = 0;
            while(true){
                child->lock();
                if(child_ver != child->version){    // the node split
                    child->unlock();
                    child_ver = child->version;
                    memory_fence();
                    child_bin *next_child = child->nextbin;
                    while(next_child && next_child->slotkey[0]<= key){
                        child = next_child;
                        rslot++;
                        child_ver = child->version;
                        memory_fence();
                        next_child = child->nextbin;
                    }
                } else {
                    break;
                }
            }
            // =====  insert  =====
            //size_t slot = find_lower_avx(child->slotkey, childslotmax, key);
            size_t slot = find_lower(child, key);
            // equal?
            if (slot < child->slotuse && key_equal(key, child->slotkey[slot])) {
                child->unlock();
                return result_t::failed;
            }
            // not full?
            if(!child->isfull()){
                std::copy_backward(child->slotkey + slot, child->slotkey + child->slotuse,
                                   child->slotkey + child->slotuse+1);
                std::copy_backward(child->slotdata + slot, child->slotdata + child->slotuse,
                                   child->slotdata + child->slotuse+1);
                child->slotkey[slot] = key;
                child->slotdata[slot] = value;
                child->slotuse++;
                // check root?
                memory_fence();
                root_bin *root = m_root;
                if(root && slot == child->slotuse-1 && rslot<=root->slotuse-1) {
                    root->lock();
                    root->slotkey[rslot] = key;
                }

                memory_fence();
                child->version++;
                if(root) root->version++;
                memory_fence();
                child->unlock();
                if(root) root->unlock();
                return result_t::ok;
            }else{
                return con_split_child_bin(child, slot, key, value);
            }
        }
        return result_t::failed;    
    }

    result_t con_split_child_bin(child_bin* child, size_t slot, const key_type &key, const data_type &value)
    {
        child_bin *curchild = child;
        child_bin *newchild = allocate_childbin();
        newchild->lock();
        unsigned int mid = (child->slotuse >> 1);
        // new child
        newchild->slotuse = child->slotuse - mid;
        std::copy(child->slotkey + mid, child->slotkey + child->slotuse,
                  newchild->slotkey);
        std::copy(child->slotdata + mid, child->slotdata + child->slotuse,
                  newchild->slotdata);
        newchild->nextbin = child->nextbin;
        newchild->prevbin = child;
        // update root 
        key_type newkey = child->slotkey[mid-1];
        memory_fence();
        root_bin *root = m_root;
        if(root==nullptr){
            root = allocate_rootbin();
            root->lock();
            root->slotkey[0] = newkey;
            root->children[0] = child;
            root->children[1] = newchild;
            root->slotuse = 1;
            memory_fence();
            if(m_root) {
                child->unlock();
                free_childbin(newchild);
                free_rootbin(root);
                return con_insert_start(key, value);
                //return result_t::retry;
            }
            m_root = root;
            memory_fence();
        } else {
            return result_t::failed;
        }
        child->slotuse = (child->slotuse >> 1);
        child->nextbin = newchild;
        if (newchild->nextbin == nullptr) {
            // assert(child == m_tailbin);
            m_tailbin = newchild;
        }
        // check if insert slot is in the split sibling node
        // special case
        if(slot == mid){
            // special case: the node was split, and the insert is at the
            // last slot of the old node. then the splitkey must be
            // updated.
            child->slotkey[slot] = key;
            child->slotdata[slot] = value;
            child->slotuse++;
            root->slotkey[0] = key;
        } else if (slot > mid) {
            slot -= child->slotuse;
            std::copy_backward(newchild->slotkey + slot, newchild->slotkey + newchild->slotuse,
                               newchild->slotkey + newchild->slotuse+1);
            std::copy_backward(newchild->slotdata + slot, newchild->slotdata + newchild->slotuse,
                               newchild->slotdata + newchild->slotuse+1);
            newchild->slotkey[slot] = key;
            newchild->slotdata[slot] = value;
            newchild->slotuse++;
        } else{
            std::copy_backward(child->slotkey + slot, child->slotkey + child->slotuse,
                               child->slotkey + child->slotuse+1);
            std::copy_backward(child->slotdata + slot, child->slotdata + child->slotuse,
                               child->slotdata + child->slotuse+1);
            child->slotkey[slot] = key;
            child->slotdata[slot] = value;
            child->slotuse++;
        }     
        memory_fence();
        root->version++;
        child->version++;
        newchild->version++;
        memory_fence();
        child->unlock();
        newchild->unlock();
        root->unlock();
        return result_t::ok;    
    }

    result_t con_insert_start_retrain(const key_type& key, const data_type& value) 
    {
        if(is_retraining){
            COUT_THIS("level bin is retraining! "<<key);
            return result_t::failed;
        }

        bin *newchild = NULL;
        key_type newkey = key_type();
        
        memory_fence();
        if(m_root) {
            //search the root
            root_bin* root = m_root;
            int slot = find_lower(root, key);
            // currnode has 
            if(root->children[slot]->slotuse < childslotmax)
                return insert_child_retrain(root->children[slot], key, value, &newkey, &newchild);
            
            // insert into prechild?
            if(slot > 0 && root->children[slot-1]->slotuse < rootslotmax) {
                child_bin* prev = static_cast<child_bin*>(root->children[slot-1]);
                child_bin* curr = static_cast<child_bin*>(root->children[slot]);
                // insert the smallest key into prevchild
                if(curr->slotkey[0] > key){
                    root->lock();
                    prev->lock();

                    prev->slotkey[prev->slotuse] = key;
                    prev->slotdata[prev->slotuse] = value;
                    prev->slotuse++;
                    root->slotkey[slot-1] = key;

                    memory_fence();
                    prev->version++;
                    root->version++;
                    memory_fence();
                    prev->unlock();
                    root->unlock();

                    return result_t::ok;
                } else if(curr->slotkey[0] < key){
                    int curslot = find_lower(curr, key);
                    // equal?
                    if (curslot < curr->slotuse && key_equal(key, curr->slotkey[curslot])) {
                        return result_t::failed;
                    }
                    root->lock();
                    prev->lock();
                    curr->lock();

                    prev->slotkey[prev->slotuse] = curr->slotkey[0];
                    prev->slotdata[prev->slotuse] = curr->slotdata[0];
                    prev->slotuse++;
                    root->slotkey[slot-1] = curr->slotkey[0];
                    
                    std::copy(curr->slotkey+1, curr->slotkey + curslot,
                              curr->slotkey);
                    std::copy(curr->slotdata + 1, curr->slotdata + curslot,
                              curr->slotdata);
                    curr->slotkey[curslot-1] = key;
                    curr->slotdata[curslot-1] = value;
                    // special case
                    if ( curslot == curr->slotuse && slot <= root->slotuse-1)
                    {
                        // special case: the node was split, and the insert is at the
                        // last slot of the old node. then the splitkey must be
                        // updated.
                        root->slotkey[slot] = key;
                    }

                    memory_fence();
                    prev->version++;
                    curr->version++;
                    root->version++;
                    memory_fence();
                    prev->unlock();
                    curr->unlock();
                    root->unlock();

                    return result_t::ok;
                } else{
                    return result_t::failed;
                }
            } else { //insert into current child
                // need train?
                if(root->isfull()){
                    //std::cout<<"need retrain!" << std::endl;
                    is_retraining = true;
                    return result_t::retrain;
                }
                result_t r = insert_child_retrain(root->children[slot], key, value, &newkey, &newchild);         
                if(newchild){
                    root->lock();

                    std::copy_backward(root->slotkey + slot, root->slotkey + root->slotuse,
                                       root->slotkey + root->slotuse+1);
                    std::copy_backward(root->children + slot, root->children + root->slotuse+1,
                                       root->children + root->slotuse+2);

                    root->slotkey[slot] = newkey;
                    root->children[slot + 1] = newchild;
                    root->slotuse++;

                    memory_fence();
                    root->version++;
                    memory_fence();
                    root->unlock();
                }
                return r;
            }
        } else {               // only occors when there is only one bin
            memory_fence();
            if(m_headbin == NULL) {
                m_headbin = m_tailbin = allocate_childbin();
            }
            child_bin *child = m_headbin;
            result_t r = insert_child_retrain(child, key, value, &newkey, &newchild);
            // create m_root
            if (newchild)
            {
                root_bin *newroot = allocate_rootbin();
                newroot->slotkey[0] = newkey;
                newroot->children[0] = child;
                newroot->children[1] = newchild;
                newroot->slotuse = 1;
                memory_fence();
                assert(m_root==nullptr);
                m_root = newroot;
            }
            return r;
        }
    }
    result_t insert_child_retrain(bin *b, const key_type& key, const data_type& value, 
                key_type* newkey, bin** newchild){
        child_bin* child = static_cast<child_bin*>(b);
        int slot = find_lower(child, key);
        // equal?
        if (slot < child->slotuse && key_equal(key, child->slotkey[slot])) {
            return result_t::failed;
        }
        // full?
        if (child->isfull())
        {
            //split
            split_child_bin_retrain(child, newkey, newchild);

            // check if insert slot is in the split sibling node
            if (slot >= child->slotuse)
            {
                slot -= child->slotuse;
                child = static_cast<child_bin*>(*newchild);
            }
        }
        child->lock();

        std::copy_backward(child->slotkey + slot, child->slotkey + child->slotuse,
                           child->slotkey + child->slotuse+1);
        std::copy_backward(child->slotdata + slot, child->slotdata + child->slotuse,
                           child->slotdata + child->slotuse+1);
        child->slotkey[slot] = key;
        child->slotdata[slot] = value;
        child->slotuse++;

        memory_fence();
        child->version++;
        memory_fence();
        child->unlock();

        // special case
        if (newchild && child != *newchild && slot == child->slotuse-1)
        {
            // special case: the node was split, and the insert is at the
            // last slot of the old node. then the splitkey must be
            // updated.
            *newkey = key;
        }
        return result_t::ok;
    }
    void split_child_bin_retrain(child_bin* child, key_type* _newkey, bin** _newchild)
    {
        assert(child->isfull());
        unsigned int mid = (child->slotuse >> 1);
        child_bin *newchild = allocate_childbin();
        newchild->slotuse = child->slotuse - mid;

        newchild->nextbin = child->nextbin;
        if (newchild->nextbin == NULL) {
            assert(child == m_tailbin);
            m_tailbin = newchild;
        }
        else {
            newchild->nextbin->prevbin = newchild;
        }
        newchild->lock();
        child->lock();

        std::copy(child->slotkey + mid, child->slotkey + child->slotuse,
                  newchild->slotkey);
        std::copy(child->slotdata + mid, child->slotdata + child->slotuse,
                  newchild->slotdata);

        child->slotuse = mid;
        child->nextbin = newchild;
        newchild->prevbin = child;
        
        memory_fence();
        child->version++;
        newchild->version++;
        memory_fence();
        newchild->unlock();
        child->unlock();

        *_newkey = child->slotkey[child->slotuse-1];
        *_newchild = newchild;
    }




// ============= scan ==============
public:
    int scan(const key_type &key, const size_t n, std::vector<std::pair<key_type, data_type>> &result) {

        child_bin* child = nullptr;
        size_t remaining = n;
        memory_fence();
        if(m_root){
            root_bin* root = m_root;
            int slot = find_lower(root, key);
            child = static_cast<child_bin*>(root->children[slot]);
        } else {
            if(m_headbin){
                child = m_headbin;
            } else {
                return remaining;
            }
        }
        assert(child!=nullptr);
        int cslot = find_lower(child, key);
        while(remaining>0 && child) {
            result.push_back(std::pair<key_type, data_type>(child->slotkey[cslot], child->slotdata[cslot]));
            remaining--;
            cslot++;
            if(cslot>=child->slotuse){
                child = child->nextbin;
                cslot = 0;
            }
        }
        return remaining;
    }




// remove from the bins
public:
    result_t remove(const key_type &key)
    {
        if(is_retraining) {
            return result_t::retrain;
        }

        memory_fence();
        if(m_root){
            root_bin* root = m_root;
            int slot = find_lower(root, key);
            child_bin* child = static_cast<child_bin*>(root->children[slot]);
            int cslot = find_lower(child, key);
            // out of scope? or not equal?
            if (cslot >= child->slotuse || !key_equal(key, child->slotkey[cslot]))
            {
                return result_t::failed;
            }

            child->lock();
            std::copy(child->slotkey + cslot+1, child->slotkey + child->slotuse,
                      child->slotkey + cslot);
            std::copy(child->slotdata + cslot+1, child->slotdata + child->slotuse,
                      child->slotdata + cslot);
            child->slotuse--;
            memory_fence();
            child->version++;
            memory_fence();
            child->unlock();

            // empty?
            if(child->slotuse == 0){
                memory_fence();
                assert(m_root!=nullptr);
                root = m_root;
                root->lock();

                // has prevchild?
                if(child->prevbin){
                    child->prevbin->nextbin = child->nextbin;
                } else{
                    m_headbin = child->nextbin;
                }
                // has nextchild?
                if(child->nextbin){
                    child->nextbin->prevbin = child->prevbin;
                } else{
                    m_tailbin = child->prevbin;
                }
                free_childbin(child);
                // remove from root
                std::copy(root->slotkey + slot+1, root->slotkey + root->slotuse,
                          root->slotkey + slot);
                std::copy(root->children + slot+1, root->children + root->slotuse+1,
                          root->children + slot);
                root->slotuse--;

                // root empty?
                if(root->slotuse == 0){
                    assert(m_headbin == m_tailbin);
                    root->unlock();
                    free_rootbin(m_root);
                    m_root = nullptr;
                    return result_t::ok;
                }
                memory_fence();
                root->version++;
                memory_fence();
                root->unlock();
                return result_t::ok;
            }
            
            // special case: remove the last key?
            if(cslot == child->slotuse){
                root->lock();
                root->slotkey[slot] = child->slotkey[child->slotuse - 1];
                memory_fence();
                root->version++;
                memory_fence();
                root->unlock();
            }
            return result_t::ok;
        } else {         // occurs only when there is only one bin
            if(m_headbin == nullptr) {
                return result_t::failed;
            }
            child_bin *child = m_headbin;
            
            int slot = find_lower(child, key);
            // out of scope? or not equal?
            if (slot >= child->slotuse || !key_equal(key, child->slotkey[slot]))
            {
                return result_t::failed;
            }
            child->lock();
            std::copy(child->slotkey + slot+1, child->slotkey + child->slotuse,
                      child->slotkey + slot);
            std::copy(child->slotdata + slot+1, child->slotdata + child->slotuse,
                      child->slotdata + slot);
            child->slotuse--;

            memory_fence();
            child->version++;
            memory_fence();
            child->unlock();

            // empty?
            if(child->slotuse == 0){
                free_childbin(child);
                m_headbin = m_tailbin = nullptr;
            }
            return result_t::ok;
        }
    }



private:
    root_bin* m_root;
    child_bin* m_headbin;
    child_bin* m_tailbin;
    key_compare m_key_less;
    allocator_type m_allocator;
    volatile bool is_retraining = false;


};    // class LevelBinCon


} // namespace aidel


#endif