#include <assert.h>
#include <algorithm>
#include "N.h"

namespace ART_OLC {

    bool N256::isFull() const {
        return false;
    }

    bool N256::isUnderfull() const {
        return count == 37;
    }

    void N256::deleteChildren(DeleteKeyFunction deleteKey) {
        for (uint64_t i = 0; i < 256; ++i) {
            if (children[i] != nullptr) {
                N::deleteChildren(children[i], deleteKey);
                N::deleteNode(children[i], deleteKey);
            }
        }
    }

    uint64_t N256::size(LoadKeyFunction loadKey) const {
        uint64_t size = sizeof(*this);
        for (uint64_t i = 0; i < 256; ++i) {
            if (children[i] != nullptr) {
                size += N::size(children[i], loadKey);
            }
        }
        return size;
    }

    void N256::insert(uint8_t key, N *val) {
        children[key] = val;
        count++;
    }

    template<class NODE>
    void N256::copyTo(NODE *n) const {
        for (int i = 0; i < 256; ++i) {
            if (children[i] != nullptr) {
                n->insert(i, children[i]);
            }
        }
    }

    bool N256::change(uint8_t key, N *n) {
        children[key] = n;
        return true;
    }

    N *N256::getChild(const uint8_t k) const {
        return children[k];
    }

    void N256::remove(uint8_t k) {
        children[k] = nullptr;
        count--;
    }

    N *N256::getAnyChild() const {
        N *anyChild = nullptr;
        for (uint64_t i = 0; i < 256; ++i) {
            if (children[i] != nullptr) {
                if (N::isLeaf(children[i])) {
                    return children[i];
                } else {
                    anyChild = children[i];
                }
            }
        }
        return anyChild;
    }

    uint64_t N256::getChildren(uint8_t start, uint8_t end, std::tuple<uint8_t, N *> *&children,
                           uint32_t &childrenCount, bool& needRestart) const {
        restart:
        needRestart = false;
        uint64_t v;
        v = readLockOrRestart(needRestart);
        if (needRestart){
            return v;
        }
        childrenCount = 0;
        for (unsigned i = start; i <= end; i++) {
            if (this->children[i] != nullptr) {
                children[childrenCount] = std::make_tuple(i, this->children[i]);
                childrenCount++;
            }
        }
        readUnlockOrRestart(v, needRestart);
        if (needRestart) goto restart;
        return v;
    }
}