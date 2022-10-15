# Workload on Sorted Data Testbed (WOSD)

WOSD is a testbed to compare (learned) indexes on various datasets and workloads, and it is generally composed of three components (i.e., workload generation, hyper-parameter tuning, performance evaluation).

## Running the testbed

We provide a number of scripts to automate things. Each is located in the `scripts` directory, but should be executed from the repository root.

- `./scripts/download.sh` downloads and stores required data from the Internet
- `./scripts/build_rmis.sh` compiles and builds the RMIs for each dataset. If you run into the error message `error: no override and no default toolchain set`, try running `rustup install stable`.
- `./scripts/download_rmis.sh` will download pre-built RMIs instead, which may be faster. You'll need to run `build_rmis.sh` if you want to measure build times on your platform.
- `./scripts/prepare.sh` constructs the single-thread workloads and compiles the testbed, and `./scripts/prepare_multithread.sh` for concurrency workloads.
- `./scripts/execute.sh` executes the testbed on single-thread workloads, storing the results in `results`, and `./scripts/execute_multithread.sh` for concurrency workloads.

Build times can be long, as we make aggressive use of templates to ensure we do not accidentally measure vtable lookup time. 

## Results

The results in `results/through-results`, `results/multithread-results`, `results/string-results` are shown in the following format.
```csv
(index name) (bulk loading time) (index size) (throughputs) (hyper-parameters)
```

The results in `results/latency-results` are shown in the following format.
```csv
(index name) (bulk loading time) (index size) (average, P50, P99, P99.9, max, standard derivation of the latency) (hyper-parameters)
```

The results in `results/errors-results` are shown in the following format.
```csv
(index name) (bulk loading time) (index size) (average, P50, P99, P99.9, max, standard derivation of the latency) (average position search overhead) (average position search overhead per operation) (average prediction error)
```
The filenames of csvs in `results` comply with the following rules: `{dataset}_ops_{operation count}_{range query ratio}_{negative lookup ratio}_{insert ratio}_({insert pattern}_)({hotspot ratio}_)({thread number}_)(mix_)({bulk loaded data size}_)results_table.csv`
