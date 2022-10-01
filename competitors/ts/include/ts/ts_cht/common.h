#pragma once

#include <cstddef>
#include <cstdint>

namespace ts_cht {

struct SearchBound {
  size_t begin;
  size_t end;  // Exclusive.
};

}  // namespace cht
