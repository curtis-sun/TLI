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
use crate::util::{Line, Point, Segment};

#[derive(PartialEq)]
enum GreedyState {
    Need2,
    Need1,
    Ready,
}

/// Performs a greedy piecewise linear regression (PLR) in an online fashion. This approach uses constant
/// time for each call to [`process`](#method.process) as well as constant space. Note that, due to the
/// greedy nature of the algorithm, more regression segments may be created than strictly needed. For
/// PLR with a minimal number of segments, please see [`OptimalPLR`](struct.OptimalPLR.html).
///
/// Each call to [`process`](#method.process) consumes a single point. Each time it is called,
/// [`process`](#method.process) returns either a [`Segment`](../struct.Segment.html) representing
/// a piece of the final regression, or `None`. If your stream of points terminates, you can call
/// [`finish`](#method.finish) to flush the buffer and return the final segment.
///
/// # Example
/// ```
/// use plr::regression::GreedyPLR;
/// // first, generate some data points...
/// let mut data = Vec::new();
///  
/// for i in 0..1000 {
///     let x = (i as f64) / 1000.0 * 7.0;
///     let y = f64::sin(x);
///     data.push((x, y));
/// }
///  
/// let mut plr = GreedyPLR::new(0.0005); // gamma = 0.0005, the maximum regression error
///  
/// let mut segments = Vec::new();
///  
/// for (x, y) in data {
///     // when `process` returns a segment, we should add it to our list
///     if let Some(segment) = plr.process(x, y) {
///         segments.push(segment);
///     }
/// }
///
/// // because we have a finite amount of data, we flush the buffer and get the potential
/// // last segment.
/// if let Some(segment) = plr.finish() {
///     segments.push(segment);
/// }
///
/// // the `segments` vector now contains all segments for this regression.
/// ```
pub struct GreedyPLR {
    state: GreedyState,
    gamma: f64,
    s0: Option<Point>,
    s1: Option<Point>,
    sint: Option<Point>,
    s_last: Option<Point>,
    rho_lower: Option<Line>,
    rho_upper: Option<Line>,
}

impl GreedyPLR {
    /// Enables performing PLR using a greedy algorithm with a fixed gamma (maximum error).
    ///
    /// # Examples
    ///
    /// To perform a greedy PLR with a maximum error of `0.05`:
    /// ```
    /// use plr::regression::GreedyPLR;
    /// let plr = GreedyPLR::new(0.05);
    /// ```
    pub fn new(gamma: f64) -> GreedyPLR {
        return GreedyPLR {
            state: GreedyState::Need2,
            gamma,
            s0: None,
            s1: None,
            sint: None,
            s_last: None,
            rho_lower: None,
            rho_upper: None,
        };
    }

    fn setup(&mut self) {
        // we have two points, initialize rho lower and rho upper.
        let gamma = self.gamma;
        let s0 = self.s0.as_ref().unwrap();
        let s1 = self.s1.as_ref().unwrap();

        self.rho_lower = Some(s0.upper_bound(gamma).line_to(&s1.lower_bound(gamma)));
        self.rho_upper = Some(s0.lower_bound(gamma).line_to(&s1.upper_bound(gamma)));

        self.sint = Some(Line::intersection(
            self.rho_lower.as_ref().unwrap(),
            self.rho_upper.as_ref().unwrap(),
        ));

        assert!(self.sint.is_some());
    }

    fn current_segment(&self, end: f64) -> Segment {
        assert!(self.state == GreedyState::Ready);

        let segment_start = self.s0.as_ref().unwrap().as_tuple().0;
        let segment_stop = end;

        let avg_slope = Line::average_slope(
            self.rho_lower.as_ref().unwrap(),
            self.rho_upper.as_ref().unwrap(),
        );

        let (sint_x, sint_y) = self.sint.as_ref().unwrap().as_tuple();
        let intercept = -avg_slope * sint_x + sint_y;
        return Segment {
            start: segment_start,
            stop: segment_stop,
            slope: avg_slope,
            intercept,
        };
    }

    fn process_pt(&mut self, pt: Point) -> Option<Segment> {
        assert!(self.state == GreedyState::Ready);

        if !(pt.above(self.rho_lower.as_ref().unwrap())
            && pt.below(self.rho_upper.as_ref().unwrap()))
        {
            // we cannot adjust either extreme slope to fit this point, we have to
            // start a new segment.
            let current_segment = self.current_segment(pt.as_tuple().0);

            self.s0 = Some(pt);
            self.state = GreedyState::Need1;
            return Some(current_segment);
        }

        // otherwise, we can adjust the extreme slopes to fit the point.
        let s_upper = pt.upper_bound(self.gamma);
        let s_lower = pt.lower_bound(self.gamma);
        if s_upper.below(self.rho_upper.as_ref().unwrap()) {
            self.rho_upper = Some(self.sint.as_ref().unwrap().line_to(&s_upper));
        }

        if s_lower.above(self.rho_lower.as_ref().unwrap()) {
            self.rho_lower = Some(self.sint.as_ref().unwrap().line_to(&s_lower));
        }

        return None;
    }

    /// Processes a single point using the greedy PLR algorithm. This function returns
    /// a new [`Segment`](../struct.Segment.html) when the current segment cannot accommodate
    /// the passed point, and returns None if the current segment could be (greedily) adjusted to
    /// fit the point.
    pub fn process(&mut self, x: f64, y: f64) -> Option<Segment> {
        let pt = Point::new(x, y);
        self.s_last = Some(pt);

        let mut returned_segment: Option<Segment> = None;

        let new_state = match self.state {
            GreedyState::Need2 => {
                self.s0 = Some(pt);
                GreedyState::Need1
            }
            GreedyState::Need1 => {
                self.s1 = Some(pt);
                self.setup();
                GreedyState::Ready
            }
            GreedyState::Ready => {
                returned_segment = self.process_pt(pt);

                if returned_segment.is_some() {
                    GreedyState::Need1
                } else {
                    GreedyState::Ready
                }
            }
        };

        self.state = new_state;
        return returned_segment;
    }

    /// Terminates the PLR process, returning a final segment if required.
    pub fn finish(self) -> Option<Segment> {
        return match self.state {
            GreedyState::Need2 => None,
            GreedyState::Need1 => {
                let s0 = self.s0.unwrap().as_tuple();
                Some(Segment {
                    start: s0.0,
                    stop: std::f64::MAX,
                    slope: 0.0,
                    intercept: s0.1,
                })
            }
            GreedyState::Ready => Some(self.current_segment(std::f64::MAX)),
        };
    }
}

#[cfg(test)]
mod test {
    use super::*;
    use crate::test_util::*;

    #[test]
    fn test_sin() {
        let mut plr = GreedyPLR::new(0.0005);
        let data = sin_data();

        let mut segments = Vec::new();

        for &(x, y) in data.iter() {
            if let Some(segment) = plr.process(x, y) {
                segments.push(segment);
            }
        }

        if let Some(segment) = plr.finish() {
            segments.push(segment);
        }

        assert_eq!(segments.len(), 77);
        verify_gamma(0.0005, &data, &segments);
    }

    #[test]
    fn test_linear() {
        let mut plr = GreedyPLR::new(0.00005);
        let data = linear_data(10.0, 25.0);

        let mut segments = Vec::new();

        for &(x, y) in data.iter() {
            if let Some(segment) = plr.process(x, y) {
                segments.push(segment);
            }
        }

        if let Some(segment) = plr.finish() {
            segments.push(segment);
        }

        assert_eq!(segments.len(), 1);
        verify_gamma(0.00005, &data, &segments);
    }

    #[test]
    fn test_precision() {
        let mut plr = GreedyPLR::new(0.00005);
        let data = precision_data();

        let mut segments = Vec::new();

        for &(x, y) in data.iter() {
            if let Some(segment) = plr.process(x, y) {
                segments.push(segment);
            }
        }

        if let Some(segment) = plr.finish() {
            segments.push(segment);
        }

        assert_eq!(segments.len(), 1);
        verify_gamma(0.00005, &data, &segments);
    }

    #[test]
    fn test_osm() {
        let mut plr = GreedyPLR::new(64.0);
        let data = osm_data();

        let mut segments = Vec::new();

        for &(x, y) in data.iter() {
            if let Some(segment) = plr.process(x, y) {
                segments.push(segment);
            }
        }

        if let Some(segment) = plr.finish() {
            segments.push(segment);
        }

        verify_gamma(64.0, &data, &segments);
    }

    #[test]
    fn test_fb() {
        let mut plr = GreedyPLR::new(64.0);
        let data = fb_data();

        let mut segments = Vec::new();

        for &(x, y) in data.iter() {
            if let Some(segment) = plr.process(x, y) {
                segments.push(segment);
            }
        }

        if let Some(segment) = plr.finish() {
            segments.push(segment);
        }

        verify_gamma(64.0, &data, &segments);
    }
}
