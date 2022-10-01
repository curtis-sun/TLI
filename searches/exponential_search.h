#pragma once
#include "search.h"

template<int record>
class ExponentialSearch : public Search<record> {
 public:
  template<typename Iterator, typename KeyType>
  static forceinline Iterator lower_bound(
    Iterator first, Iterator last,
		const KeyType& lookup_key, Iterator start,
    std::function<KeyType(Iterator)> at = [](Iterator it)->KeyType{
      return static_cast<KeyType>(*it);
    },
    std::function<bool(const KeyType&, const KeyType&)> less = [](const KeyType& key1, const KeyType& key2)->bool{
      return key1 < key2;
    }) {
      record_start();
      if (first == last) { 
        record_end(first, first);
        return first; 
      }

      size_t bound = 1;
      Iterator l = start, r = start;
      if (start != last && less(at(start), lookup_key)) {
        size_t size = std::distance(l, last) >> 1;
        std::advance(r, bound);
        while (bound <= size && less(at(r), lookup_key)) {
          bound <<= 1;
          l = r;
          std::advance(r, bound);
        }
        if (bound > size){
          r = last;
        }
        ++l;
      } else {
        size_t size = (std::distance(first, l) + 1) >> 1;
        std::advance(l, -bound);
        while (bound <= size && !(less(at(l), lookup_key))) {
          bound <<= 1;
          r = l;
          std::advance(l, -bound);
        }
        if (bound > size) {
          l = first;
        }
        else{
          ++l;
        }
      }
      auto it = BranchingBinarySearch<0>::lower_bound(l, r, lookup_key, l, at);

      record_end(start, it);
      return it;
    }

  template<typename Iterator, typename KeyType>
  static forceinline Iterator upper_bound(
    Iterator first, Iterator last,
		const KeyType& lookup_key, Iterator start,
    std::function<KeyType(Iterator)> at = [](Iterator it)->KeyType{
      return static_cast<KeyType>(*it);
    },
    std::function<bool(const KeyType&, const KeyType&)> less = [](const KeyType& key1, const KeyType& key2)->bool{
      return key1 < key2;
    }) {
      record_start();
      if (first == last) { 
        record_end(first, first);
        return first; 
      }

      size_t bound = 1;
      Iterator l = start, r = start;
      if (start == last || less(lookup_key, at(start))) {
        size_t size = (std::distance(first, l) + 1) >> 1;
        std::advance(l, -bound);
        while (bound <= size && less(lookup_key, at(l))) {
          bound <<= 1;
          r = l;
          std::advance(l, -bound);
        }
        if (bound > size) {
          l = first;
        }
        else{
          ++l;
        }
      } else {
        size_t size = std::distance(l, last) >> 1;
        std::advance(r, bound);
        while (bound <= size && !(less(lookup_key, at(r)))) {
          bound <<= 1;
          l = r;
          std::advance(r, bound);
        }
        if (bound > size){
          r = last;
        }
        ++l;
      }
      auto it = BranchingBinarySearch<0>::upper_bound(l, r, lookup_key, l, at);

      record_end(start, it);
      return it;
    }

  static std::string name() { return "ExponentialSearch"; }
};
