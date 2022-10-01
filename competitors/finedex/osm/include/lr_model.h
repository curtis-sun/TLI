#pragma once

#include <vector>
#include "util.h"

namespace finedex {

template <class key_t>
class LinearRegressionModel{
 private:
  size_t Epsilon = 0;
  double slope;
  double intercept;
 
 public:
  explicit LinearRegressionModel(double s, double i, size_t e) : slope(s), intercept(i), Epsilon(e) {}

  ApproxPos predict(const key_t &k, const size_t size) const {
    auto pos = int64_t(slope * k) + intercept;
    auto lo = SUB_EPS(pos, Epsilon);
    auto hi = ADD_EPS(pos, Epsilon, size);
    lo = lo>hi? hi:lo;
    return {static_cast<size_t>(ceil(pos)), static_cast<size_t>(ceil(lo)), static_cast<size_t>(ceil(hi))};
  }

  inline void print() const {
    LOG(2) << "[slope, intercept]: "<<slope << ", " <<intercept;
  }

  // fixme
  void train(const typename std::vector<key_t>::const_iterator &it, size_t size)
  {
    std::vector<key_t> trainkeys(size);
    std::vector<size_t> positions(size);
    for(size_t i=0; i<size; i++){
      trainkeys[i]=*(it+i);
      positions[i] = i;
    }
    train(trainkeys, positions);
  }

  void train(const std::vector<key_t> &keys, const std::vector<size_t> &positions)
  {
      assert(keys.size() == positions.size());
      if (keys.size() == 0) return;

      std::vector<double> model_keys(keys.size());
      std::vector<double *> key_ptrs(keys.size());
      for (size_t i = 0; i < keys.size(); i++) {
          model_keys[i] = keys[i];
      }

      if (positions.size() == 0) return;
      if (positions.size() == 1) {
          slope = 0;
          intercept = positions[0];
          return;
      }
      // use multiple dimension LR when running tpc-c
      double x_expected = 0.0, y_expected = 0.0, xy_expected = 0.0,
             x_square_expected = 0.0;
      for (size_t key_i = 0; key_i < positions.size(); key_i++) {
          double key = static_cast<double>(model_keys[key_i]);
          x_expected += key;
          y_expected += positions[key_i];
          x_square_expected += key * key;
          xy_expected += key * positions[key_i];
      }
      assert(x_expected>0);
      assert(y_expected>0);
      assert(x_square_expected>0);
      assert(xy_expected>0);

      x_expected /= positions.size();
      y_expected /= positions.size();
      x_square_expected /= positions.size();
      xy_expected /= positions.size();

      slope = (xy_expected - x_expected * y_expected) /
                   (x_square_expected - x_expected * x_expected);
      intercept = (x_square_expected * y_expected - x_expected * xy_expected) /
                   (x_square_expected - x_expected * x_expected);
      Epsilon = max_error(keys, positions);
  }

  size_t max_error(const std::vector<key_t> &keys, const std::vector<size_t> &positions)
  {
      size_t max = 0;
      for (size_t key_i = 0; key_i < keys.size(); ++key_i) {
          long long int pos_actual = positions[key_i];
          long long int pos_pred = predict_pos(keys[key_i]);
          int error = std::abs(pos_actual - pos_pred);
          if (error > max) {
              max = error;
          }
      }
      return max;
  }

  size_t predict_pos(const key_t &key) const{
      double model_key = key;
      double res = slope * model_key + intercept;
      return res > 0 ? res : 0;
  }

  inline double get_slope() {return slope;}
  inline double get_intercept() { return intercept; }
  inline size_t get_epsilon() { return Epsilon; }
};

} // finedex