#pragma once
#include "search.h"
#include "linear_search.h"

template <int record>
class InterpolationSearch : public Search<record> {
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
      
      if (start != last && at(start) < lookup_key){
        Iterator mid = start;
        ++mid;
        it = lower_bound_(mid, last, lookup_key, at);
      }
      else{
        it = lower_bound_(first, start, lookup_key, at);
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

      if (start == last || lookup_key < at(start)){
        it = upper_bound_(first, start, lookup_key, at);
      }
      else{
        Iterator mid = start;
        ++mid;
        it = upper_bound_(mid, last, lookup_key, at);
      }
      record_end(start, it);
      return it;
    }

  static std::string name() { return "InterpolationSearch"; }

private:
  template<typename Iterator, typename KeyType>
  static forceinline Iterator lower_bound_(
    Iterator first, Iterator last,
		const KeyType& lookup_key, std::function<KeyType(Iterator)> at) {
      if (first == last) {
        return first;
      }

      --last;
      while(at(first) < at(last) && at(first) < lookup_key && !(at(last) < lookup_key)){
        double rel_position =
          (double)(lookup_key - at(first)) / (double)(at(last) - at(first));
        size_t mid_offset = rel_position * std::distance(first, last);
        Iterator mid = first;
        std::advance(mid, mid_offset);

        if (at(mid) < lookup_key){
          first = ++mid;
        } else if (lookup_key < at(mid)){
          last = --mid;
        }
        else {
          while (mid != first && at(mid) == lookup_key){
            --mid;
          }
          if (at(mid) < lookup_key){
            ++mid;
          }
          return mid;
        }
      }

      if (!(at(first) < lookup_key)){
        return first;
      }
      ++last;

      return last;
    }

  template<typename Iterator, typename KeyType>
  static forceinline Iterator upper_bound_(
    Iterator first, Iterator last,
		const KeyType& lookup_key, std::function<KeyType(Iterator)> at) {
      if (first == last) {
        return first;
      }

      --last;
      while(at(first) < at(last) && !(lookup_key < at(first)) && lookup_key < at(last)){
        double rel_position =
          (double)(lookup_key - at(first)) / (double)(at(last) - at(first));
        size_t mid_offset = rel_position * std::distance(first, last);
        Iterator mid = first;
        std::advance(mid, mid_offset);

        if (at(mid) < lookup_key){
          first = ++mid;
        } else if (lookup_key < at(mid)){
          last = --mid;
        }
        else {
          ++last;
          while (mid != last && at(mid) == lookup_key){
            ++mid;
          }
          return mid;
        }
      }

      if (lookup_key < at(first)){
        return first;
      }
      ++last;

      return last;
    }
};