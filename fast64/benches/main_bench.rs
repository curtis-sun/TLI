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
 
use fast64::Fast64;
use criterion::{criterion_group, criterion_main, Criterion};
use rand::prelude::*;

pub fn criterion_benchmark(c: &mut Criterion) {
    {
        let keys: Vec<u64> = (0..4096).collect();
        let values: Vec<u64> = (0..4096).collect();
        let tree = Fast64::new(keys, values);

        println!("Tree has depth {} and size {}", tree.depth(), tree.size());
        
        c.bench_function("small safe", |b| b.iter(|| {
            for i in 0..4095 {
                tree.slow_lookup(i);
            }
        }));

        c.bench_function("small unsafe", |b| b.iter(|| {
            for i in 0..4095 {
                tree.fast_lookup(i);
            }
        }));
    }

    {
        let keys: Vec<u64> = (0..200000000).collect();
        let values: Vec<u64> = (0..200000000).collect();
        let tree = Fast64::new(keys, values);

        println!("Tree has depth {} and size {}", tree.depth(), tree.size());
        
        let mut rng = rand::thread_rng();
        let queries: Vec<u64> = (0..2000)
            .map(|_| rng.gen_range(0, 200000000)).collect();
                

        
        c.bench_function("large safe", |b| b.iter(|| {
            for &i in queries.iter() {
                tree.slow_lookup(i);
            }
        }));
        
        c.bench_function("large unsafe", |b| b.iter(|| {
            for &i in queries.iter() {
                tree.fast_lookup(i);
            }
        }));
    }
    
}


criterion_group!(benches, criterion_benchmark);
criterion_main!(benches);
