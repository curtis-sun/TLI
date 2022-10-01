// < begin copyright >
// Copyright Ryan Marcus 2019
//
// This file is part of plr.
//
// plr is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// plr is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with plr.  If not, see <http://www.gnu.org/licenses/>.
//
// < end copyright >
use crate::data::*;
use crate::util::*;
use std::collections::VecDeque;
use superslice::*;

pub fn sin_data() -> Vec<(f64, f64)> {
    let mut data = Vec::new();

    for i in 0..1000 {
        let x = (i as f64) / 1000.0 * 7.0;
        let y = f64::sin(x);
        data.push((x, y));
    }

    return data;
}

pub fn linear_data(slope: f64, intercept: f64) -> Vec<(f64, f64)> {
    let mut data = Vec::new();

    for i in 0..10 {
        let x = (i as f64) / 1000.0;
        let y = x * slope + intercept;
        data.push((x, y));
    }

    return data;
}

pub fn precision_data() -> Vec<(f64, f64)> {
    let mut data = Vec::new();

    for i in 0..1000 {
        let x = ((i as f64) / 1000.0) * f64::powi(2.0, 60);

        let y = i as f64;

        data.push((x, y));
    }

    return data;
}

pub fn osm_data() -> Vec<(f64, f64)> {
    return OSM_DATA
        .iter()
        .map(|&(x, y)| (x as f64, y as f64))
        .collect();
}

pub fn fb_data() -> Vec<(f64, f64)> {
    return FB_DATA.iter().map(|&(x, y)| (x as f64, y as f64)).collect();
}

pub fn verify_gamma(gamma: f64, data: &[(f64, f64)], segments: &[Segment]) {
    let mut seg_q = VecDeque::new();

    for segment in segments {
        seg_q.push_back(segment);
    }

    for &(x, y) in data {
        while seg_q.front().unwrap().stop <= x {
            seg_q.pop_front();
        }

        let seg = seg_q.front().unwrap();

        assert!(seg.start <= x);
        assert!(seg.stop >= x);

        let line = Line::new(seg.slope, seg.intercept);
        let pred = line.at(x).y;

        assert!(
            f64::abs(pred - y) <= gamma,
            "Prediction of {} was not within gamma ({}) of true value {}",
            pred,
            gamma,
            y
        );
    }
}

fn spline_interpolate(pt: f64, knots: &[(f64, f64)]) -> (f64, f64) {
    let upper_idx = usize::min(
        knots.len() - 1,
        knots.upper_bound_by(|x| x.0.partial_cmp(&pt).unwrap()),
    );
    let lower_idx = upper_idx - 1;

    return Point::from_tuple(knots[lower_idx])
        .line_to(&Point::from_tuple(knots[upper_idx]))
        .at(pt)
        .as_tuple();
}

pub fn verify_gamma_splines(gamma: f64, data: &[(f64, f64)], pts: &[(f64, f64)]) {
    println!("{:?}", pts);
    for &(x, y) in data {
        let pred = spline_interpolate(x, pts);
        assert!(
            f64::abs(pred.1 - y) <= gamma,
            "Prediction of {} was not within gamma ({}) of true value {}",
            pred.1,
            gamma,
            y
        );
    }
}

pub fn verify_gamma_splines_cast(gamma: f64, data: &[(f64, f64)], pts: &[(f64, f64)]) {
    println!("{:?}", pts);
    for &(x, y) in data {
        let x = (x as u64) as f64;
        let pred = spline_interpolate(x, pts);
        let pred = pred.1 as u64;
        assert!(
            f64::abs(pred as f64 - y) <= gamma,
            "Prediction of {} was not within gamma ({}) of true value {}",
            pred,
            gamma,
            y
        );
    }
}
