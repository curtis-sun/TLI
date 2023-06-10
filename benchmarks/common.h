#include "searches/branching_binary_search.h"
#include "searches/interpolation_search.h"
#include "searches/exponential_search.h"
#include "searches/linear_search_avx.h"

#ifdef FAST_MODE
#define INSTANTIATE_TEMPLATES_(func_name, type_name, track_errors)                              \
  template void func_name<BranchingBinarySearch<track_errors>>(                                 \
      tli::Benchmark<type_name>&, const std::vector<int>&)
#else
#define INSTANTIATE_TEMPLATES_(func_name, type_name, track_errors)                              \
  template void func_name<BranchingBinarySearch<track_errors>>(                                 \
      tli::Benchmark<type_name>&, bool, const std::vector<int>&);                               \
  template void func_name<ExponentialSearch<track_errors>>(                                     \
      tli::Benchmark<type_name>&, bool, const std::vector<int>&);                               \
  template void func_name<LinearSearch<track_errors>>(                                          \
      tli::Benchmark<type_name>&, bool, const std::vector<int>&);                               \
  template void func_name<LinearAVX<type_name, track_errors>>(                                  \
      tli::Benchmark<type_name>&, bool, const std::vector<int>&);                               \
  template void func_name<InterpolationSearch<track_errors>>(                                   \
      tli::Benchmark<type_name>&, bool, const std::vector<int>&);                               \
  template void func_name<track_errors>(                                                        \
      tli::Benchmark<type_name>&, const std::string&)
#endif

#ifdef FAST_MODE
#define INSTANTIATE_TEMPLATES_(func_name, type_name, track_errors)                              \
  template void func_name<BranchingBinarySearch<track_errors>>(                                 \
      tli::Benchmark<type_name>&, const std::vector<int>&, const std::string&)
#else
#define INSTANTIATE_TEMPLATES_RMI_(func_name, type_name, track_errors)                          \
  template void func_name<BranchingBinarySearch<track_errors>>(                                 \
      tli::Benchmark<type_name>&, bool, const std::vector<int>&, const std::string&);           \
  template void func_name<ExponentialSearch<track_errors>>(                                     \
      tli::Benchmark<type_name>&, bool, const std::vector<int>&, const std::string&);           \
  template void func_name<LinearSearch<track_errors>>(                                          \
      tli::Benchmark<type_name>&, bool, const std::vector<int>&, const std::string&);           \
  template void func_name<LinearAVX<type_name, track_errors>>(                                  \
      tli::Benchmark<type_name>&, bool, const std::vector<int>&, const std::string&);           \
  template void func_name<InterpolationSearch<track_errors>>(                                   \
      tli::Benchmark<type_name>&, bool, const std::vector<int>&, const std::string&);           \
  template void func_name<track_errors>(                                                        \
      tli::Benchmark<type_name>&, const std::string&)
#endif

#define INSTANTIATE_TEMPLATES(func_name, type_name)                                             \
  INSTANTIATE_TEMPLATES_(func_name, type_name, 0);                                              \
  INSTANTIATE_TEMPLATES_(func_name, type_name, 1);

#define INSTANTIATE_TEMPLATES_MULTITHREAD(func_name, type_name)                                 \
  INSTANTIATE_TEMPLATES_(func_name, type_name, 0);                                              \
  INSTANTIATE_TEMPLATES_(func_name, type_name, 1);                                              \
  INSTANTIATE_TEMPLATES_(func_name, type_name, 2)
