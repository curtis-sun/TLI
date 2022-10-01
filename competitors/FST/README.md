# Fast Succinct Tries (Succinct Range Filter)

**FST** is a fast and compact data structure. This is the source code for our
[SIGMOD best paper](http://www.cs.cmu.edu/~huanche1/publications/surf_paper.pdf).

## Install Dependencies
    sudo apt-get install build-essential cmake libgtest.dev
    cd /usr/src/gtest
    sudo cmake CMakeLists.txt
    sudo make
    sudo cp *.a /usr/lib

## Build
    git submodule init
    git submodule update
    mkdir build
    cd build
    cmake ..
    make -j

## Simple Example
A simple example can be found [here](https://github.com/efficient/FST/blob/master/simple_example.cpp). To run the example:
```
g++ -mpopcnt -std=c++11 simple_example.cpp
./a.out
```
Note that the key list passed to the FST constructor must be SORTED.

## Run Unit Tests
    make test

## License
Copyright 2018, Carnegie Mellon University

Licensed under the [Apache License](https://github.com/efficient/FST/blob/master/LICENSE).
