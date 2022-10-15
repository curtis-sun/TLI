# Workload on Sorted Data Testbed (WOSD)

WOSD is a testbed to compare (learned) indexes on various datasets and workloads, and it is generally composed of three components (i.e., workload generation, hyper-parameter tuning, performance evaluation).

## Dependencies

One dependency that should be emphasized is [Intel MKL](https://software.intel.com/en-us/mkl), used when testing the performance of [XIndex and SIndex](https://ipads.se.sjtu.edu.cn:1312/opensource/xindex). The detailed steps of installation can be found [here](https://software.intel.com/en-us/articles/installing-intel-free-libs-and-python-apt-repo).

Generally, the dependencies can be installed in the following steps.

```shell
$ cd /tmp
$ wget https://apt.repos.intel.com/intel-gpg-keys/GPG-PUB-KEY-INTEL-SW-PRODUCTS-2019.PUB
$ apt-key add GPG-PUB-KEY-INTEL-SW-PRODUCTS-2019.PUB
$ rm GPG-PUB-KEY-INTEL-SW-PRODUCTS-2019.PUB

$ sh -c 'echo deb https://apt.repos.intel.com/mkl all main > /etc/apt/sources.list.d/intel-mkl.list'
$ apt-get update
$ apt-get install -y intel-mkl-2019.0-045

$ apt -y install zstd python3-pip m4 cmake clang libboost-all-dev
$ pip3 install --user numpy scipy
$ curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
$ source $HOME/.cargo/env
```

After the installation, the following two lines in `CMakeLists.txt` may require modification.

```cmake
set(MKL_LINK_DIRECTORY "/opt/intel/mkl/lib/intel64")
set(MKL_INCLUDE_DIRECTORY "/opt/intel/mkl/include")
```

## Running the testbed

We provide a number of scripts to automate things. Each is located in the `scripts` directory, but should be executed from the repository root.

- `./scripts/download.sh` downloads and stores required data from the Internet
- `./scripts/build_rmis.sh` compiles and builds the RMIs for each dataset. If you run into the error message `error: no override and no default toolchain set`, try running `rustup install stable`.
- `./scripts/download_rmis.sh` will download pre-built RMIs instead, which may be faster. You'll need to run `build_rmis.sh` if you want to measure build times on your platform.
- `./scripts/prepare.sh` constructs the single-thread workloads and compiles the testbed, and `./scripts/prepare_multithread.sh` for concurrency workloads.
- `./scripts/execute.sh` executes the testbed on single-thread workloads, storing the results in `results`, and `./scripts/execute_multithread.sh` for concurrency workloads.

Build times can be long, as we make aggressive use of templates to ensure we do not accidentally measure vtable lookup time. 

## Results

The results in `results/through-results` are obtained in single-thread workloads, `results/multithread-results` in concurrency workloads, and `results/string-results` for string indexes. They are shown in the following format.
```txt
(index name) (bulk loading time) (index size) (throughputs) (hyper-parameters)
```

The results in `results/latency-results` are obtained measuring latencies in single-thread workload, and are shown in the following format.
```txt
(index name) (bulk loading time) (index size) (average, P50, P99, P99.9, max, standard derivation of the latency) (hyper-parameters)
```

The results in `results/errors-results` are obtained measuring position searches, and are shown in the following format.
```txt
(index name) (bulk loading time) (index size) (average, P50, P99, P99.9, max, standard derivation of the latency) (average position search overhead) (average position search overhead per operation) (average prediction error) (hyper-parameters)
```

The filenames of csvs in `results` comply with the following rule.
```txt
{dataset}_ops_{operation count}_{range query ratio}_{negative lookup ratio}_{insert ratio}_({insert pattern}_)({hotspot ratio}_)({thread number}_)(mix_)({bulk-loaded data size}_)results_table.csv
```
