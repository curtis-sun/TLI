// < begin copyright > 
// Copyright Ryan Marcus 2020
// 
// This file is part of fast64.
// 
// fast64 is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// fast64 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with fast64.  If not, see <http://www.gnu.org/licenses/>.
// 
// < end copyright > 
#include <stdint.h>

void fast_lookup64(const uint64_t** internal_pages, const uint64_t num_internal_pages,
                   const uint64_t* leaf_page,
                   const uint64_t query,
                   uint64_t* const out1, uint64_t* const out2);

void fast_lookup32(const uint32_t** internal_pages, const uint32_t num_internal_pages,
                   const uint32_t* leaf_page,
                   const uint32_t query,
                   uint32_t* const out1, uint32_t* const out2);
