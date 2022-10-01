/*
  Fast Architecture Sensitive Tree layout for binary search trees
  (Kim et. al, SIGMOD 2010)
  implementation by Viktor Leis, TUM, 2012
  notes:
  -keys are 4 byte integers
  -SSE instructions are used for comparisons
  -huge memory pages (2MB)
  -page blocks store 4 levels of cacheline blocks
  -cacheline blocks store 15 keys and are 64-byte aligned
  -the parameter K results in a tree size of (2^(16+K*4))
 */

#include <sys/mman.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdint.h>
#include <iostream>
#include <emmintrin.h>
#include <cassert>
#include <climits>
#include <string.h>
#include <algorithm>
#include <vector>
#include <random>
#include <utility>
#include <cmath>

const unsigned SIMD_BYTE = 256 / 8;
const unsigned CACHE_LINE_BYTE = 64;
const unsigned PAGE_BYTE = (1 << 21);

template<class KeyType>
class FAST {
public:
    FAST(): depth(0), len(0), v(nullptr), size_in_byte_(0){}

    ~FAST() {
        munmap(v, size_in_byte_);
    }

    void buildFAST(KeyType l[], size_t _len) {
        // create array of appropriate size
        len = _len;
        depth = std::floor(std::log2(len)) + 1;
        unsigned depth_div_page = (depth - 1) / PAGE_CACHE_DEPTH;
        unsigned depth_page = depth_div_page * PAGE_CACHE_DEPTH;
        unsigned long long n = ((pow(depth_div_page) - 1) << PAGE_DEPTH) + (pow(depth_page + depth - depth_page + 1));
        size_in_byte_ = sizeof(KeyType) * n;
        v = (KeyType*)malloc_huge(size_in_byte_);

        // build FAST
        size_t offset = 0;
        size_t actual_len = pow(depth);
        for (unsigned level = 0; level < depth; level += PAGE_CACHE_DEPTH) {
            const size_t level_num = pow(level);
            const size_t chunk = actual_len / level_num;
            for (unsigned cl = 0; cl < level_num; ++cl){
                if (level + PAGE_CACHE_DEPTH < depth){
                    storeFASTpage(v, offset, l, cl * chunk, (cl + 1) * chunk, PAGE_CACHE_DEPTH);
                    offset += PAGE_SIZE;
                }
                else{
                    offset = storeFASTpage(v, offset, l, cl * chunk, (cl + 1) * chunk, depth - level);
                }
            }
        }
        assert(offset == n);
    }

    size_t lower_bound(const KeyType& key_q) const {
        __m256i xmm_key_q;
        init_simd(key_q, xmm_key_q);

        KeyType* page_v = v;
        size_t page_address = 0;

        for (unsigned page_level = 0; page_level < depth; page_level += PAGE_CACHE_DEPTH) {
            size_t page_offset = 0;
            unsigned page_depth = std::min(depth - page_level, PAGE_CACHE_DEPTH);
            unsigned page_actual_depth = (page_level + PAGE_CACHE_DEPTH < depth) ? PAGE_DEPTH : page_depth + 1;
            KeyType* cache_v = page_v + (page_address << page_actual_depth);

            for (unsigned cache_level = 0; cache_level < page_depth; cache_level += CACHE_LINE_DEPTH) {
                size_t cache_offset = 0;
                unsigned cache_depth = std::min(CACHE_LINE_DEPTH, page_depth - cache_level);
                KeyType* simd_v = cache_v + (page_offset << cache_depth);

                for (unsigned simd_level = 0; simd_level < cache_depth; simd_level += SIMD_DEPTH) {
                    unsigned simd_depth = std::min(SIMD_DEPTH, cache_depth - simd_level);
                    KeyType* child_v = simd_v + (cache_offset << simd_depth) - cache_offset;
                    size_t child_index;

                    if (simd_depth == SIMD_DEPTH){
                        child_index = search_simd(child_v, xmm_key_q);
                    }
                    else{
                        child_index = search_scalar(child_v, key_q, simd_depth);
                    }

                    cache_offset = (cache_offset << simd_depth) + child_index;
                    simd_v += ((pow(simd_depth) - 1) << simd_level);
                }

                page_offset = (page_offset << cache_depth) + cache_offset;
                cache_v += pow(cache_level + cache_depth);
            }

            page_address = (page_address << page_depth) + page_offset;
            page_v += pow(page_level + page_actual_depth);
        }
        return std::min(page_address, len);
    }

    unsigned long long size_in_byte() const {
        return size_in_byte_;
    }
private:
    const unsigned SIMD_SIZE = SIMD_BYTE / sizeof(KeyType);
    const unsigned CACHE_LINE_SIZE = CACHE_LINE_BYTE / sizeof(KeyType);
    const unsigned PAGE_SIZE = PAGE_BYTE / sizeof(KeyType);
    const unsigned SIMD_DEPTH = __builtin_ctz(SIMD_SIZE);
    const unsigned CACHE_LINE_DEPTH = __builtin_ctz(CACHE_LINE_SIZE);
    const unsigned PAGE_DEPTH = __builtin_ctz(PAGE_SIZE);
    const unsigned PAGE_CACHE_DEPTH = (PAGE_DEPTH - 1) / CACHE_LINE_DEPTH * CACHE_LINE_DEPTH;
    const unsigned SIMD_MASK = pow(SIMD_SIZE - 1) - 1;

    unsigned depth;
    size_t len;
    KeyType* v;
    unsigned long long size_in_byte_;

    void* malloc_huge(size_t size) {
        void* p=mmap(NULL,size,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
        #if __linux__
            madvise(p,size,MADV_HUGEPAGE);
        #endif
        return p;
    }

    static inline size_t pow(unsigned exponent) {
        return 1ULL << exponent;
    }

    static inline size_t median(size_t i,size_t j) {
        return i + (j - 1 - i) / 2;
    }

    size_t storeSIMDblock(KeyType v[], size_t offset, KeyType l[], size_t i, size_t j, unsigned levels) {
        for (unsigned level = 0; level < levels; ++level) {
            const size_t level_num = pow(level);
            const size_t chunk = (j - i) / level_num;
            for (unsigned cl = 0; cl < level_num; ++cl){
                size_t idx = median(i + cl * chunk, i + (cl + 1) * chunk - 1);
                v[offset++] = idx < len ? l[idx] : std::numeric_limits<KeyType>::max();
            }
        }
        return offset;
    }

    size_t storeCachelineBlock(KeyType v[], size_t offset, KeyType l[], size_t i, size_t j, unsigned levels) {
        size_t org_offset = offset;
        for (unsigned level = 0; level < levels; level += SIMD_DEPTH) {
            const size_t level_num = pow(level);
            const size_t chunk = (j - i) / level_num;
            for (unsigned cl = 0; cl < level_num; ++cl){
                offset = storeSIMDblock(v, offset, l, i + cl * chunk, i + (cl + 1) * chunk, std::min(levels - level, SIMD_DEPTH));
            }
        }
        return org_offset + pow(levels);
    }

    size_t storeFASTpage(KeyType v[], size_t offset, KeyType l[], size_t i, size_t j, unsigned levels) {
        size_t org_offset = offset;
        for (unsigned level = 0; level < levels; level += CACHE_LINE_DEPTH) {
            const size_t level_num = pow(level);
            const size_t chunk = (j - i) / level_num;
            for (unsigned cl = 0; cl < level_num; ++cl){
                offset = storeCachelineBlock(v, offset, l, i + cl * chunk, i + (cl + 1) * chunk, std::min(CACHE_LINE_DEPTH, levels - level));
            }
        }
        assert(offset <= org_offset + pow(levels + 1));
        return org_offset + pow(levels + 1);
    }

    inline unsigned maskToIndex(unsigned bitmask) const {
        bitmask &= SIMD_MASK;
        if constexpr (SIMD_BYTE / sizeof(KeyType) == 4){
            static unsigned table[8]={0, 9, 1, 2, 9, 9, 9, 3};
            return table[bitmask];
        }
        if (bitmask){
            return __builtin_popcount(bitmask);
        }
        return 0;
    }

    size_t search_simd(KeyType* v, const __m256i& xmm_key_q) const {
        __m256i xmm_tree = _mm256_loadu_si256((__m256i*)v);
        if constexpr (sizeof(KeyType) == 4){
            __m256i xmm_mask = _mm256_cmpgt_epu32(xmm_key_q, xmm_tree);
            unsigned index = _mm256_movemask_ps(_mm256_castsi256_ps(xmm_mask));
            return maskToIndex(index);
        }
        if constexpr (sizeof(KeyType) == 8){
            __m256i xmm_mask = _mm256_cmpgt_epu64(xmm_key_q, xmm_tree);
            unsigned index = _mm256_movemask_pd(_mm256_castsi256_pd(xmm_mask));
            return maskToIndex(index);
        }
        exit(-1);
    }

    size_t search_scalar(KeyType* v, const KeyType& key_q, unsigned simd_depth) const {
        size_t child_index = 0;
        for (unsigned child_level = 0; child_level < simd_depth; ++child_level){
            child_index = (child_index << 1) + (*(v + child_index) < key_q);
            v += pow(child_level);
        }
        return child_index;
    }

    void init_simd(const KeyType& key_q, __m256i& xmm_key_q) const {
        if constexpr (sizeof(KeyType) == 4){
            xmm_key_q=_mm256_set1_epi32(key_q);
            return;
        }
        if constexpr (sizeof(KeyType) == 8){
            xmm_key_q=_mm256_set1_epi64x(key_q);
            return;
        }
        exit(-1);
    }
};