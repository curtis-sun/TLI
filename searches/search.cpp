#include "search.h"
                                           
uint64_t Search<1>::timing;                                    
size_t Search<1>::search_num;                                  
uint64_t Search<1>::sum_search_bound;  
size_t Search<1>::research_num;

std::atomic<uint64_t> Search<2>::timing;            
std::atomic<size_t> Search<2>::search_num;          
std::atomic<uint64_t> Search<2>::sum_search_bound;
size_t Search<2>::research_num;