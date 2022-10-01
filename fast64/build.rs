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
 


fn main() {
    let have_avx512 = is_x86_feature_detected!("avx512f");
    
    let avx512_flag = if have_avx512 {
        "1"
    } else {
        "0"
    };

    let vanilla_flag = if !have_avx512 {
        "1"
    } else {
        "0"
    };

    
    cc::Build::new()
        .flag("-march=native")
        .define("AVX512", avx512_flag)
        .define("VANILLA", vanilla_flag)
        .file("c/lookup.c")
        .compile("lookup");
}
