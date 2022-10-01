#pragma once
#include "search.h"

template<int record>
class LinearSearch: public Search<record> {
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

      Iterator it = start;
      if (start != last && less(at(start), lookup_key)){
        ++it;
        while(it != last && less(at(it), lookup_key)) {
          ++it;
        }
      }
      else{
        while(it != first){
          --it;
          if (less(at(it), lookup_key)){
            break;
          }
        }
        if (less(at(it), lookup_key)){
          ++it;
        }
      }

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

      Iterator it = start;
      if (start == last || less(lookup_key, at(start))){
        while(it != first){
          --it;
          if (!(less(lookup_key, at(it)))){
            break;
          }
        }
        if (!(less(lookup_key, at(it)))){
          ++it;
        }
      }
      else{
        ++it;
        while(it != last && !(less(lookup_key, at(it)))) {
          ++it;
        }
      }

      record_end(start, it);
      return it;
    }

  static std::string name() {
      return "LinearSearch";
  }
};
