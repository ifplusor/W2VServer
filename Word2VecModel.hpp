//
// Created by James on 2017/9/3.
//

#ifndef __W2V_WORD2VEC_MODEL_HPP__
#define __W2V_WORD2VEC_MODEL_HPP__

#include "types.h"
#include "Sampler.hpp"

class Word2VecModel {
 public:
  Word2VecModel(size_t vocabLen, size_t vecLen, size_t sampleNum)
      : fVocabLen(vocabLen), fVecLen(vecLen), fNegative(sampleNum) {
    syn0 = new real[fVecLen * fVocabLen];
    syn1neg = new real[fVecLen * fVocabLen];
  }

  virtual ~Word2VecModel() {
    delete syn0;
    delete syn1neg;
  }

  virtual void Step(size_t center, const size_t *context, size_t len);

 protected:
  Sampler *fSampler;

  size_t fVocabLen, fVecLen, fNegative;
  double fAlpha;

  real *syn0, *syn1neg;
};

#endif //__W2V_WORD2VEC_MODEL_HPP__
