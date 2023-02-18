#include <assert.h>
#include <algorithm>
#include "N.h"
#include <emmintrin.h> // x86 SSE intrinsics

namespace ART_OLC {

    bool N16::isFull() const {
        return count == 16;
    }

    bool N16::isUnderfull() const {
        return count == 3;
    }

    void N16::insert(uint8_t key, N *n) {
        uint8_t keyByteFlipped = flipSign(key);
        __m128i cmp = _mm_cmplt_epi8(_mm_set1_epi8(keyByteFlipped), _mm_loadu_si128(reinterpret_cast<__m128i *>(keys)));
        uint16_t bitfield = _mm_movemask_epi8(cmp) & (0xFFFF >> (16 - count));
        unsigned pos = bitfield ? ctz(bitfield) : count;
        memmove(keys + pos + 1, keys + pos, count - pos);
        memmove(children + pos + 1, children + pos, (count - pos) * sizeof(uintptr_t));
        keys[pos] = keyByteFlipped;
        children[pos] = n;
        count++;
    }

    template<class NODE>
    void N16::copyTo(NODE *n) const {
        for (unsigned i = 0; i < count; i++) {
            n->insert(flipSign(keys[i]), children[i]);
        }
    }

    bool N16::change(uint8_t key, N *val) {
        N **childPos = const_cast<N **>(getChildPos(key));
        assert(childPos != nullptr);
        *childPos = val;
        return true;
    }

    N *const *N16::getChildPos(const uint8_t k) const {
        __m128i cmp = _mm_cmpeq_epi8(_mm_set1_epi8(flipSign(k)),
                                     _mm_loadu_si128(reinterpret_cast<const __m128i *>(keys)));
        unsigned bitfield = _mm_movemask_epi8(cmp) & ((1 << count) - 1);
        if (bitfield) {
            return &children[ctz(bitfield)];
        } else {
            return nullptr;
        }
    }

    N *const *N16::getChildGePos(const uint8_t k) const {
        __m128i cmp = _mm_xor_si128(_mm_cmplt_epi8(_mm_loadu_si128(reinterpret_cast<const __m128i *>(keys)), _mm_set1_epi8(flipSign(k))), _mm_set1_epi8(char(-1)));
        unsigned bitfield = _mm_movemask_epi8(cmp) & (0xFFFF >> (16 - count));
        if (bitfield) {
            return &children[ctz(bitfield)];
        } else {
            return nullptr;
        }
    }

    N *const *N16::getChildGtPos(const uint8_t k) const {
        __m128i cmp = _mm_cmpgt_epi8(_mm_loadu_si128(reinterpret_cast<const __m128i *>(keys)), _mm_set1_epi8(flipSign(k)));
        unsigned bitfield = _mm_movemask_epi8(cmp) & (0xFFFF >> (16 - count));
        if (bitfield) {
            return &children[ctz(bitfield)];
        } else {
            return nullptr;
        }
    }

    N *N16::getChild(const uint8_t k) const {
        N *const *childPos = getChildPos(k);
        if (childPos == nullptr) {
            return nullptr;
        } else {
            return *childPos;
        }
    }

    void N16::remove(uint8_t k) {
        N *const *leafPlace = getChildPos(k);
        assert(leafPlace != nullptr);
        std::size_t pos = leafPlace - children;
        memmove(keys + pos, keys + pos + 1, count - pos - 1);
        memmove(children + pos, children + pos + 1, (count - pos - 1) * sizeof(N *));
        count--;
        assert(getChild(k) == nullptr);
    }

    N *N16::getAnyChild() const {
        for (int i = 0; i < count; ++i) {
            if (N::isLeaf(children[i])) {
                return children[i];
            }
        }
        return children[0];
    }

    void N16::deleteChildren(DeleteKeyFunction deleteKey) {
        for (std::size_t i = 0; i < count; ++i) {
            N::deleteChildren(children[i], deleteKey);
            N::deleteNode(children[i], deleteKey);
        }
    }

    uint64_t N16::size(LoadKeyFunction loadKey) const {
        uint64_t size = sizeof(*this);
        for (std::size_t i = 0; i < count; ++i) {
            size += N::size(children[i], loadKey);
        }
        return size;
    }

    uint64_t N16::getChildren(uint8_t start, uint8_t end, std::tuple<uint8_t, N *> *&children,
                          uint32_t &childrenCount, bool& needRestart) const {
        restart:
        needRestart = false;
        uint64_t v;
        v = readLockOrRestart(needRestart);
        if (needRestart){
            return v;
        }
        childrenCount = 0;
        auto startPos = getChildGePos(start);
        auto endPos = getChildGtPos(end);
        if (startPos == nullptr) {
            startPos = this->children;
        }
        if (endPos == nullptr) {
            endPos = this->children + (count - 1);
        }
        else{
            if (endPos == this->children){
                readUnlockOrRestart(v, needRestart);
                if (needRestart) goto restart;
                return v;
            }
            -- endPos;
        }
        for (auto p = startPos; p <= endPos; ++p) {
            children[childrenCount] = std::make_tuple(flipSign(keys[p - this->children]), *p);
            childrenCount++;
        }
        readUnlockOrRestart(v, needRestart);
        if (needRestart) goto restart;
        return v;
    }
}