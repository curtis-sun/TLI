use criterion::{criterion_group, criterion_main, Criterion};
use plr::regression::{GreedyPLR, OptimalPLR};

pub fn criterion_benchmark(c: &mut Criterion) {
    let mut data = Vec::new();

    for idx in 0..2000 {
        let x = idx as f64 / 2000.0;
        let y = f64::sin(x);

        data.push((x, y));
    }

    c.bench_function("optimal 2000", |b| {
        b.iter(|| {
            let mut plr = OptimalPLR::new(0.0005);

            for &(x, y) in data.iter() {
                plr.process(x, y);
            }

            plr.finish();
        })
    });

    c.bench_function("greedy 2000", |b| {
        b.iter(|| {
            let mut plr = GreedyPLR::new(0.0005);

            for &(x, y) in data.iter() {
                plr.process(x, y);
            }

            plr.finish();
        })
    });
}

criterion_group!(benches, criterion_benchmark);
criterion_main!(benches);
