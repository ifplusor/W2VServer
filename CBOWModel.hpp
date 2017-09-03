//
// Created by James on 2017/9/2.
//

#ifndef __W2V_MODEL_CBOW_HPP__
#define __W2V_MODEL_CBOW_HPP__

#include "Word2VecModel.hpp"
#include "ExpTable.hpp"

class CBOWModel : public Word2VecModel {
 public:
  CBOWModel(size_t vocabLen, size_t vecLen, size_t sampleNum)
      : Word2VecModel(vocabLen, vecLen, sampleNum) {}

  virtual ~CBOWModel() {}

  void Step(size_t center, const size_t *context, size_t len) override;
};

void CBOWModel::Step(size_t center, const size_t *context, size_t len) {
  ll a, b, c, d;
  ll l1, l2, label;
  real f, g;
  size_t lastWord, target;
  ull random;

  real *neu1 = (real *) calloc(fVecLen, sizeof(real));
  real *neu1e = (real *) calloc(fVecLen, sizeof(real));

  for (c = 0; c < fVecLen; c++) {
    neu1[c] = neu1e[c] = 0.;
  }

  for (a = 0; a < len; a++) {
    lastWord = context[a];
    l1 = lastWord * fVecLen;
    for (c = 0; c < fVecLen; c++)
      neu1[c] += syn0[c + l1];
  }

  for (d = 0; d < fNegative + 1; d++) {
    if (d == 0) { // 正样本
      target = center;
      label = 1;
    } else { // 负样本
      target = fSampler->Sampling();
      if (target == center) continue;
      label = 0;
    }
    l2 = target * fVecLen;
    f = 0;
    for (c = 0; c < fVecLen; c++)
      f += neu1[c] * syn1neg[c + l2];

    g = (label - sigmoid(f)) * fAlpha;

    for (c = 0; c < fVecLen; c++) neu1e[c] += g * syn1neg[c + l2];
    for (c = 0; c < fVecLen; c++) syn1neg[c + l2] += g * neu1[c];
  }

  // hidden -> in
  for (a = 0; a < len; a++) {
    lastWord = context[c];
    if (lastWord == 0) continue;
    for (c = 0; c < fVecLen; c++)
      syn0[c + lastWord * fVecLen] += neu1e[c];
  }
}

#endif //__W2V_MODEL_CBOW_HPP__
