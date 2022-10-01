# PLR (piecewise linear regression)

[![Rust](https://github.com/RyanMarcus/plr/actions/workflows/rust.yml/badge.svg)](https://github.com/RyanMarcus/plr/actions/workflows/rust.yml) [![crates.io](https://img.shields.io/crates/v/plr.svg)](https://crates.io/crates/plr)

Rust implementation of the greedy and optimal error-bounded PLR algorithms described in:

> Qing Xie, Chaoyi Pang, Xiaofang Zhou, Xiangliang Zhang, and Ke Deng. 2014. Maximum error-bounded Piecewise Linear Representation for online stream approximation. The VLDB Journal 23, 6 (December 2014), 915–937. DOI: https://doi.org/10.1007/s00778-014-0355-0

Error-bounded piecewise linear regression is the task of taking a set of datapoints and finding a piecewise linear function that approximates each datapoint within a fixed bound. See the [crate documentation](https://docs.rs/plr/) for more information.

![PLR at various error levels](https://rmarcus.info/images/plr.png)
