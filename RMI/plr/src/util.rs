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

use approx::*;
use rug::Float;
use serde::{Serialize, Deserialize};

#[derive(Debug, Clone, Copy, Serialize, Deserialize)]
pub struct Point {
    pub x: f64,
    pub y: f64,
}

#[derive(Debug)]
pub struct Line {
    a: f64,
    b: f64,
}

/// A single segment of a PLR. The `start` field is inclusive, the `stop` field is exclusive.
/// The `slope` and `intercept` field can be used in a linear model: in other words, for some
/// `x` such that `start <= x < stop`, the prediction is `slope * x + intercept`.
#[derive(Debug, Serialize, Deserialize)]
pub struct Segment {
    pub start: f64,
    pub stop: f64,
    pub slope: f64,
    pub intercept: f64,
}

impl Line {
    #[cfg(test)]
    pub fn new(slope: f64, intercept: f64) -> Line {
        return Line {
            a: slope,
            b: intercept,
        };
    }

    fn as_tuple(&self) -> (f64, f64) {
        return (self.a, self.b);
    }

    pub fn intersection(l1: &Line, l2: &Line) -> Point {
        let (a, c) = l1.as_tuple();
        let (b, d) = l2.as_tuple();

        // handle floating point precision issues when both slopes are very
        // small numbers
        if relative_eq!(a, b) {
            let a_f = Float::with_val(128, a);
            let b_f = Float::with_val(128, b);
            let c_f = Float::with_val(128, c);
            let d_f = Float::with_val(128, d);

            let denom_f: Float = Float::with_val(128, &a_f - &b_f);

            let x_val =
                Float::with_val(128, (d_f.clone() - c_f.clone()) / denom_f.clone()).to_f64();
            let y_val = ((a_f * d_f - b_f * c_f) / denom_f).to_f64();

            return Point::new(x_val, y_val);
        }

        let denom = a - b;
        let x_val = (d - c) / denom;
        let y_val = (a * d - b * c) / denom;

        return Point::new(x_val, y_val);
    }

    pub fn average_slope(l1: &Line, l2: &Line) -> f64 {
        if relative_eq!(l1.a, l2.a) {
            // use higher precision
            let a1 = Float::with_val(128, l1.a);
            let a2 = Float::with_val(128, l2.a);
            let avg = ((a1 + a2) / Float::with_val(128, 2.0)).to_f64();
            return avg;
        }

        // min + max to avoid precision loss
        return (f64::min(l1.a, l2.a) + f64::max(l1.a, l2.a)) / 2.0;
    }

    pub fn slope(&self) -> f64 {
        return self.a;
    }

    pub fn at(&self, x: f64) -> Point {
        return Point::new(x, self.a * x + self.b);
    }
}

impl Point {
    pub fn new(x: f64, y: f64) -> Point {
        return Point { x, y };
    }

    #[cfg(test)]
    pub fn from_tuple(pt: (f64, f64)) -> Point {
        return Point::new(pt.0, pt.1);
    }

    pub fn as_tuple(&self) -> (f64, f64) {
        return (self.x, self.y);
    }

    pub fn slope_to(&self, other: &Point) -> f64 {
        // handle floating point precision issues when both x coords are very
        // large (or very small, although this is uncommon) numbers
        if relative_eq!(self.x, other.x) {
            let x1_f = Float::with_val(128, self.x);
            let y1_f = Float::with_val(128, self.y);
            let x2_f = Float::with_val(128, other.x);
            let y2_f = Float::with_val(128, other.y);

            let res = ((y1_f - y2_f) / (x1_f - x2_f)).to_f64();
            return res;
        }

        return (self.y - other.y) / (self.x - other.x);
    }

    pub fn line_to(&self, other: &Point) -> Line {
        let a = self.slope_to(other);

        debug_assert!(!f64::is_nan(a), "slope from {:?} to {:?} was NaN", self, other);

        let b = -a * self.x + self.y;
        return Line { a, b };
    }

    pub fn above(&self, line: &Line) -> bool {
        return self.y > line.at(self.x).y;
    }

    pub fn below(&self, line: &Line) -> bool {
        return self.y < line.at(self.x).y;
    }

    pub fn upper_bound(&self, gamma: f64) -> Point {
        // check float precision
        debug_assert!(
            !relative_eq!(self.y, self.y + gamma),
            "Gamma value of {} and encountered Y value of {} won't work in 64-bit!",
            gamma,
            self.y
        );
        return Point {
            x: self.x,
            y: self.y + gamma,
        };
    }

    pub fn lower_bound(&self, gamma: f64) -> Point {
        // check float precision
        debug_assert!(
            !relative_eq!(self.y, self.y - gamma),
            "Gamma value of {} and encountered Y value of {} won't work in 64-bit!",
            gamma,
            self.y
        );
        return Point {
            x: self.x,
            y: self.y - gamma,
        };
    }
}

#[cfg(test)]
mod test {
    use super::*;
    use approx::*;

    #[test]
    fn test_slope() {
        let p1 = Point::new(1.0, 3.0);
        let p2 = Point::new(5.0, 6.0);

        assert_relative_eq!(p1.slope_to(&p2), p2.slope_to(&p1));
        assert_relative_eq!(p1.slope_to(&p2), 0.75);
    }

    #[test]
    fn test_line() {
        let p1 = Point::new(1.0, 3.0);
        let p2 = Point::new(2.0, 6.0);

        let line1 = p1.line_to(&p2);
        let line2 = p2.line_to(&p1);

        assert_relative_eq!(line1.a, line2.a);
        assert_relative_eq!(line1.b, line2.b);

        assert_relative_eq!(line1.a, 3.0);
        assert_relative_eq!(line1.b, 0.0);
    }

    #[cfg(debug)]
    #[test]
    #[should_panic]
    fn test_line_ident() {
        let p1 = Point::new(1.0, 3.0);
        let p2 = Point::new(1.0, 6.0);

        p1.line_to(&p2);
    }

    #[test]
    fn test_intersection() {
        let p1 = Point::new(1.0, 3.0);
        let p2 = Point::new(2.0, 6.0);
        let line1 = p1.line_to(&p2);

        let p3 = Point::new(8.0, -100.0);
        let line2 = p1.line_to(&p3);

        let intersection = Line::intersection(&line1, &line2);

        assert_relative_eq!(intersection.x, p1.x);
        assert_relative_eq!(intersection.y, p1.y);
    }

    #[test]
    fn test_above_below() {
        let p1 = Point::new(1.0, 3.0);
        let p2 = Point::new(2.0, 6.0);
        let line1 = p1.line_to(&p2);

        let above = Point::new(1.5, 10.0);
        let below = Point::new(1.5, -10.0);

        assert!(above.above(&line1));
        assert!(below.below(&line1));
    }
}
