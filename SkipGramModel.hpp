//
// Created by James on 2017/9/2.
//

#ifndef __W2V_MODEL_SKIPGRAM_HPP__
#define __W2V_MODEL_SKIPGRAM_HPP__

#include "Word2VecModel.hpp"
#include "ExpTable.hpp"

class SkipGramModel : public Word2VecModel {
 public:
  SkipGramModel(Sampler *sampler, size_t vocabLen, size_t vecLen,
                size_t sampleNum, double alpha)
      : Word2VecModel(sampler, vocabLen, vecLen, sampleNum, alpha) {}

  ~SkipGramModel() override = default;

  void Step(size_t center, const size_t *context, size_t len) override;
};

void SkipGramModel::Step(size_t center, const size_t *context, size_t len) {
  size_t a, b, c, d;
  size_t l1, l2;
  real f, g, label;
  size_t lastWord, target;

  real *neu1 = (real *) calloc(fVecLen, sizeof(real));
  real *neu1e = (real *) calloc(fVecLen, sizeof(real));

  for (c = 0; c < fVecLen; c++) {
    neu1[c] = neu1e[c] = 0.;
  }

  for (a = 0; a < len; a++) {
    lastWord = context[a];
    if (lastWord == 0) continue; // 不在词表中

    l1 = lastWord * fVecLen;

    for (c = 0; c < fVecLen; c++)
      neu1[c] = syn0[c + l1];

    for (c = 0; c < fVecLen; c++) neu1e[c] = 0;

    for (d = 0; d < fNegative + 1; d++) {
      if (d == 0) {
        target = center;
        label = 1;
      } else {
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

    // Learn weights input -> hidden
    for (c = 0; c < fVecLen; c++) syn0[c + l1] += neu1e[c];
  }
}

#endif //__W2V_MODEL_SKIPGRAM_HPP__
