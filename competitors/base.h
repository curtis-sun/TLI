#pragma once

#include "../util.h"
#include "searches/search.h"

template<class KeyType>
class Base {
 public:
  uint64_t Build(const std::vector<KeyValue<KeyType>>&, size_t) {
    return 0;
  }

  size_t EqualityLookup(const KeyType&, uint32_t) const {
    return util::NOT_FOUND;
  }

  uint64_t RangeQuery(const KeyType&, const KeyType&, uint32_t) const {
    return 0;
  }
  
  void Insert(const KeyValue<KeyType>&, uint32_t) {}

  std::string name() const { return "Unknown"; }

  std::size_t size() const { return 0; }

  bool applicable(bool, bool, bool, bool, const std::string&) const {
    return true;
  }

  std::vector<std::string> variants() const { 
    return std::vector<std::string>();
  }

  double searchAverageTime() const { return 0; }
  double searchLatency(uint64_t op_cnt) const { return 0; }
  double searchBound() const { return 0; }
  void initSearch() {}
  uint64_t runMultithread(void *(* func)(void *), FGParam *params) { return 0; }
};

template<class KeyType, class SearchClass>
class Competitor: public Base<KeyType> {
 public:
  double searchAverageTime() const { return SearchClass::searchAverageTime(); }
  double searchLatency(uint64_t op_cnt) const { return (double)SearchClass::searchTotalTime() / op_cnt; }
  double searchBound() const { return SearchClass::searchBound(); }
  void initSearch() { SearchClass::initSearch(); }
};
