# A Critical Analysis of Recursive Model Indexes
Code used for the [arXiv report](https://arxiv.org/abs/2106.16166).

## Build
First clone the repository including all submodules.
```sh
git clone --recursive https://github.com/BigDataAnalyticsGroup/analysis-rmi.git
cd analysis-rmi
```
Then download the datasets and generate the source files of the RMI reference
implementation.
```sh
scripts/download_data.sh
scripts/rmi_ref/prepare_rmi_ref.sh
```
Finally, the project can then be built as follows.
```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
bin/example
```

## Example
```c++
// Initialize random number generator.
using key_type = uint64_t;
std::mt19937 gen(42);
std::uniform_int_distribution<key_type> key_distrib(0, 1UL << 48);
auto rand = [&gen, &key_distrib] { return key_distrib(gen); };

// Create 1M random keys.
std::size_t n_keys = 1e7;
std::vector<key_type> keys(n_keys);
std::generate(keys.begin(), keys.end(), rand);
std::sort(keys.begin(), keys.end());

// Build a two-layer RMI.
using layer1_type = rmi::LinearSpline;
using layer2_type = rmi::LinearRegression;
std::size_t layer2_size = 2UL << 16;
rmi::RmiLAbs<key_type, layer1_type, layer2_type> rmi(keys, layer2_size);

// Pick a key.
std::uniform_int_distribution<std::size_t> uniform_distrib(0, n_keys - 1);
key_type key = keys[uniform_distrib(gen)];

// Perform a lookup.
auto range = rmi.search(key);
auto pos = std::lower_bound(keys.begin() + range.lo, keys.begin() + range.hi, key);
std::cout << "Key " << key << " is located at position "
          << std::distance(keys.begin(), pos) << '.' << std::endl;
```

## Reproducing Experimental Results
We provide the following experiments from our paper.
* `rmi_segmentation`: Compute statistical properties on the segment sizes
  resulting from various root models (Section 5.1).
* `rmi_errors`: Compute statistical properties on the prediction errors of a
  wide range of RMI configurations (Section 5.2).
* `rmi_intervals`: Compute statistical properties on the error interval sizes
  of a wide range of RMI configurations (Section 5.3).
* `rmi_lookup`: Measure lookup times for a wide range of RMI configurations
  (Section 6).
* `rmi_build`: Measure build times for a wide range of RMI configurations and
  compare against the reference implementation (Section 7).
* `rmi_guideline`: Measure lookup times for a wide range of RMI configurations
  and compare against configurations resulting from our guideline (Section 8).
* `index_comparison`: Compare several indexes in terms of lookup time and build
  time (Section 9).

Below, we explain step by step how to reproduce our experimental results.

### Preliminaries
The following tools are required to reproduce our results.
* C++ compiler supporting C++17.
* `bash>=4`: run shell scripts.
* `cmake>=3.2`: build configuration.
* `md5sum`: validate the datasets.
* `rust`: generate reference RMIs from
  [learnedsystems/RMI](https://github.com/learnedsystems/RMI).
* `timeout`: abort experiments of slow configurations.
* `wget`: download the datasets.
* `zstd`: decompress the datasets.

In the following, we assume that all scripts are run from the root directory of
this repository. If you want to plot the results, install the corresponding
Python requirements.
```sh
pip install -r requirements.txt
```

### Running And Plotting a Single Experiment
We provide a script for running each experiment with the exact same
configuration used in the paper. To run experiment `<experiment>`, simply
execute the corresponding script `scripts/run_<experiment>.sh`, e.g., to
reproduce the experiment `index_comparison` proceed as follows.
```sh
scripts/run_index_comparison.sh
```

Depending on the hardware, experiments involving measurements of lookup time
might run several days. Results will be written to `results/<experiment>.csv`
in csv format with an appropriate header.

Afterwards, the results can be plotted by running
`scripts/plot_<experiment>.py`, e.g., to plot the results of the experiment
`index_comparison` proceed as follows.
```sh
scripts/plot_index_comparison.py
```
Note that this will visualize _all_ results of the experiment. To reproduce the
paper plots, execute the Python script with argument `--paper`.

The plots will be prefixed by the experiment name and placed in `results/`.

### Running and Plotting All Experiments at Once
To reproduce all experiments at once, run the script `scripts/run_all.sh`.
Executing all experiments will take several days. Results will be written to
`results/<experiment>.csv` in csv format with an appropriate header. Plots can
be produced as described above.

Afterwards, all results can be visualized by executing the script
`scripts/plot_all.sh`. To reproduce only the plots from the paper, execute the
script `scripts/plot_paper.sh`. The resulting plots will be prefixed by the
experiment name and place in `results/`.

## Documentation
Code documentation can be generated using `doxygen` by running the following command.
```sh
doxygen Doxyfile
```
The code documentation will be placed in `doxy/html/`.

## Cite
```
@misc{maltry2021critical,
    title={A Critical Analysis of Recursive Model Indexes},
    author={Marcel Maltry and Jens Dittrich},
    year={2021},
    eprint={2106.16166},
    archivePrefix={arXiv},
    primaryClass={cs.DB}
}
```
