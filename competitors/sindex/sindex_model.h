/*
 * The code is part of the SIndex project.
 *
 *    Copyright (C) 2020 Institute of Parallel and Distributed Systems (IPADS),
 * Shanghai Jiao Tong University. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "mkl.h"
#include "mkl_lapacke.h"

#if !defined(SINDEX_MODEL_H)
#define SINDEX_MODEL_H

namespace sindex {

static const size_t DESIRED_TRAINING_KEY_N = 10000000;

inline void model_prepare(const std::vector<double *> &model_key_ptrs,
                          const std::vector<size_t> &positions, double *weights,
                          size_t feature_len) {
  assert(model_key_ptrs.size() == positions.size());
  // set weights to all zero
  for (size_t w_i = 0; w_i < feature_len + 1; w_i++) weights[w_i] = 0;
  if (positions.size() == 0) return;
  if (positions.size() == 1) {
    weights[feature_len] = positions[0];
    return;
  }

  if (feature_len == 1) {
    double x_expected = 0, y_expected = 0, xy_expected = 0,
           x_square_expected = 0;
    for (size_t key_i = 0; key_i < positions.size(); key_i++) {
      double key = model_key_ptrs[key_i][0];
      x_expected += key;
      y_expected += positions[key_i];
      x_square_expected += key * key;
      xy_expected += key * positions[key_i];
    }
    x_expected /= positions.size();
    y_expected /= positions.size();
    x_square_expected /= positions.size();
    xy_expected /= positions.size();

    weights[0] = (xy_expected - x_expected * y_expected) /
                 (x_square_expected - x_expected * x_expected);
    weights[1] = (x_square_expected * y_expected - x_expected * xy_expected) /
                 (x_square_expected - x_expected * x_expected);
  } else {
    // trim down samples to avoid large memory usage
    size_t step = 1;
    if (model_key_ptrs.size() > DESIRED_TRAINING_KEY_N) {
      step = model_key_ptrs.size() / DESIRED_TRAINING_KEY_N;
    }

    // we only fit with useful features
    std::vector<size_t> useful_feat_index;
    for (size_t feat_i = 0; feat_i < feature_len; feat_i++) {
      double first_val = model_key_ptrs[0][feat_i];
      for (size_t key_i = 0; key_i < model_key_ptrs.size(); key_i += step) {
        if (model_key_ptrs[key_i][feat_i] != first_val) {
          useful_feat_index.push_back(feat_i);
          break;
        }
      }
    }

    if (model_key_ptrs.size() != 1 && useful_feat_index.size() == 0) {
      COUT_N_EXIT("all feats are the same");
    }

    size_t useful_feat_n = useful_feat_index.size();
    int m = model_key_ptrs.size() / step;  // number of samples
    int n = useful_feat_n + 1;             // number of features
    std::vector<double> a(m * n, 0);
    std::vector<double> b(std::max(m, n), 0);
    std::vector<double> s(std::max(1, std::min(m, n)), 0);

    for (int sample_i = 0; sample_i < m; ++sample_i) {
      for (size_t useful_f_i = 0; useful_f_i < useful_feat_n; useful_f_i++) {
        a[sample_i * n + useful_f_i] =
            model_key_ptrs[sample_i * step][useful_feat_index[useful_f_i]];
      }
      a[sample_i * n + useful_feat_n] = 1;
      b[sample_i] = positions[sample_i * step];
      assert(sample_i * step < model_key_ptrs.size());
    }

    lapack_int rank = 0;
    int fitting_res =
        LAPACKE_dgelss(LAPACK_ROW_MAJOR, m, n, 1 /* nrhs */, a.data(),
                       n /* lda */, b.data(), 1 /* ldb */, s.data(), -1, &rank);

    if (fitting_res != 0) {
      COUT_N_EXIT("fitting_res: " << fitting_res);
    }

    // set weights of useful features
    for (size_t useful_f_i = 0; useful_f_i < useful_feat_n; useful_f_i++) {
      weights[useful_feat_index[useful_f_i]] = b[useful_f_i];
    }
    weights[feature_len] = b[n - 1];  // set bias
  }
}

inline size_t model_predict(double *weights, const double *model_key,
                            size_t feature_len) {
  if (feature_len == 1) {
    double res = weights[0] * model_key[0] + weights[1];
    return res > 0 ? res : 0;
  } else {
    double res = dot_product(weights, model_key, feature_len);
    res += weights[feature_len];
    return res > 0 ? res : 0;
  }
}

}  // namespace sindex

#endif  // SINDEX_MODEL_H
