use criterion::{criterion_group, criterion_main, Criterion};
use plr::spline::greedy;

pub fn criterion_benchmark(c: &mut Criterion) {
    let mut data = Vec::new();

    for idx in 0..2000 {
        let x = idx as f64 / 2000.0;
        let y = f64::sin(x);

        data.push((x, y));
    }

    c.bench_function("spline greedy 2000", |b| {
        b.iter(|| {
            let _res = greedy(&data, 0.0005);
        })
    });
}

criterion_group!(benches, criterion_benchmark);
criterion_main!(benches);
