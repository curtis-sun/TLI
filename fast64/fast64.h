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
extern "C" {
  struct Fast64;
  struct Fast64* create_fast64(const uint64_t* keys, uint64_t num_keys,
                               const uint64_t* values, uint64_t num_values);
  void lookup_fast64(struct Fast64* tree, uint64_t key,
                     uint64_t* out1, uint64_t* out2);
  void destroy_fast64(struct Fast64* tree);
  uint64_t size_fast64(struct Fast64* tree);

  struct Fast32;
  struct Fast32* create_fast32(const uint32_t* keys, uint32_t num_keys,
                               const uint32_t* values, uint32_t num_values);
  void lookup_fast32(struct Fast32* tree, uint32_t key,
                     uint32_t* out1, uint32_t* out2);
  void destroy_fast32(struct Fast32* tree);
  uint64_t size_fast32(struct Fast32* tree);

}
