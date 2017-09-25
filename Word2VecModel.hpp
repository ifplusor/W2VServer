//
// Created by James on 2017/9/3.
//

#ifndef __W2V_WORD2VEC_MODEL_HPP__
#define __W2V_WORD2VEC_MODEL_HPP__

#include "types.h"
#include "Sampler.hpp"

struct neu1_layer {
  real *neu1, *neu1e;
  size_t len;
};

neu1_layer *alloc_layer(size_t len) {
  auto layer = new neu1_layer;
  layer->len = len;
  layer->neu1 = static_cast<real *>(calloc(layer->len, sizeof(real)));
  layer->neu1e = static_cast<real *>(calloc(layer->len, sizeof(real)));
  return layer;
}

void release_layer(void *v) {
  auto *p = static_cast<neu1_layer *>(v);
  delete[] p->neu1;
  delete[] p->neu1e;
  delete  p;
}

class Word2VecModel {
 public:
  static void Initial() {
#if __PTHREADS__
    pthread_key_create(&sNeu1Key, release_layer);
    // TODO: process error
#endif
  }

  virtual ~Word2VecModel() {
    delete syn0;
    delete syn1neg;
  }

  /**
   * @brief once dp, update model arguments
   * @todo synchronized
   */
  virtual void Step(size_t center, const size_t *context, size_t len) = 0;

 protected:
  Word2VecModel(Sampler<Word> *sampler, size_t vocabLen, size_t vecLen,
                size_t sampleNum, double alpha)
      : fSampler(sampler), fVocabLen(vocabLen), fVecLen(vecLen),
        fNegative(sampleNum), fAlpha(alpha) {
    syn0 = new real[fVecLen * fVocabLen];
    syn1neg = new real[fVecLen * fVocabLen];
  }

  Sampler<Word> *fSampler;

  size_t fVocabLen, fVecLen, fNegative;
  double fAlpha;

  real *syn0, *syn1neg;

#if __PTHREADS__
  static pthread_key_t sNeu1Key;
#endif
};

#endif //__W2V_WORD2VEC_MODEL_HPP__
