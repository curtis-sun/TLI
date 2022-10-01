#pragma once
#include "search.h"

template<int record>
class BranchingBinarySearch : public Search<record> {
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
      Iterator it;

      if (start != last && less(at(start), lookup_key)){
        Iterator mid = start;
        ++mid;
        it = lower_bound_(mid, last, lookup_key, at, less);
      }
      else{
        it = lower_bound_(first, start, lookup_key, at, less);
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
      Iterator it;
      
      if (start == last || less(lookup_key, at(start))){
        it = upper_bound_(first, start, lookup_key, at, less);
      }
      else{
        Iterator mid = start;
        ++mid;
        it = upper_bound_(mid, last, lookup_key, at, less);
      }
      record_end(start, it);
      return it;
    }
  
  static std::string name() { return "BinarySearch"; }

 private:
  template<typename Iterator, typename KeyType>
  static forceinline Iterator lower_bound_(
    Iterator first, Iterator last,
		const KeyType& lookup_key, 
    std::function<KeyType(Iterator)> at = [](Iterator it)->KeyType{
      return static_cast<KeyType>(*it);
    },
    std::function<bool(const KeyType&, const KeyType&)> less = [](const KeyType& key1, const KeyType& key2)->bool{
      return key1 < key2;
    }) {
      size_t __len = std::distance(first, last);

      while (__len > 0){
    	  size_t __half = __len >> 1;
    	  Iterator __middle = first;
    	  std::advance(__middle, __half);
    	  if (less(at(__middle), lookup_key)){
    	      first = __middle;
    	      ++first;
    	      __len = __len - __half - 1;
  	    }
    	  else
    	    __len = __half;
    	}
     
      return first;
    }

  template<typename Iterator, typename KeyType>
  static forceinline Iterator upper_bound_(
    Iterator first, Iterator last,
		const KeyType& lookup_key,
    std::function<KeyType(Iterator)> at = [](Iterator it)->KeyType{
      return static_cast<KeyType>(*it);
    },
    std::function<bool(const KeyType&, const KeyType&)> less = [](const KeyType& key1, const KeyType& key2)->bool{
      return key1 < key2;
    }) {
      size_t __len = std::distance(first, last);

      while (__len > 0){
    	  size_t __half = __len >> 1;
    	  Iterator __middle = first;
    	  std::advance(__middle, __half);
    	  if (less(lookup_key, at(__middle)))
    	    __len = __half;
    	  else{
    	      first = __middle;
    	      ++first;
    	      __len = __len - __half - 1;
  	    }
    	}

      return first;
    }
};
