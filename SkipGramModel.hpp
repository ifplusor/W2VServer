//
// Created by James on 2017/9/2.
//

#ifndef __W2V_MODEL_SKIPGRAM_HPP__
#define __W2V_MODEL_SKIPGRAM_HPP__

#include "Word2VecModel.hpp"
#include "ExpTable.hpp"

class SkipGramModel : public Word2VecModel {
 public:
  SkipGramModel(Sampler<Word> *sampler, size_t vocabLen, size_t vecLen,
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

  if (len <= 0) return;

#if USE_TSD && __PTHREADS__
  auto layer = static_cast<neu1_layer *>(pthread_getspecific(sNeu1Key));
  if (layer == nullptr) {
    layer = alloc_layer(fVecLen);
    pthread_setspecific(sNeu1Key, layer);
  } else if (layer->len < fVecLen) {
    auto new_layer = alloc_layer(fVecLen);
    pthread_setspecific(sNeu1Key, new_layer);
    release_layer(layer);
    layer = new_layer;
  }
  real *neu1 = layer->neu1;
  real *neu1e = layer->neu1e;
#else
  real neu1[kMaxVectorLength], neu1e[kMaxVectorLength];
//  neu1 = (real *) calloc(fVecLen, sizeof(real));
//  neu1e = (real *) calloc(fVecLen, sizeof(real));
#endif

  for (a = 0; a < len; a++) {
    lastWord = context[a];
    if (lastWord == 0) continue; // 不在词表中

    l1 = lastWord * fVecLen;

#if USE_BLAS
    blas_copy(static_cast<const int>(fVecLen), syn0 + l1, 1, neu1, 1);
#else
    for (c = 0; c < fVecLen; c++)
      neu1[c] = syn0[c + l1];
#endif

    for (c = 0; c < fVecLen; c++) neu1e[c] = 0.0;

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

#if USE_BLAS
      f = blas_dot(static_cast<const int>(fVecLen), neu1, 1, syn1neg + l2, 1);
#else
      f = 0;
      for (c = 0; c < fVecLen; c++)
        f += neu1[c] * syn1neg[c + l2];
#endif

      g = (label - sigmoid(f)) * fAlpha;

#if USE_BLAS
      blas_axpy(static_cast<const int>(fVecLen), g, syn1neg + l2, 1, neu1e, 1);
      blas_axpy(static_cast<const int>(fVecLen), g, neu1, 1, syn1neg + l2, 1);
#else
      for (c = 0; c < fVecLen; c++) neu1e[c] += g * syn1neg[c + l2];
      for (c = 0; c < fVecLen; c++) syn1neg[c + l2] += g * neu1[c];
#endif
    }

    // Learn weights input -> hidden
#if USE_BLAS
    blas_axpy(static_cast<const int>(fVecLen), 1, neu1e, 1, syn0 + l1, 1);
#else
    for (c = 0; c < fVecLen; c++) syn0[c + l1] += neu1e[c];
#endif
  }

#if USE_TSD && __PTHREADS__
#else
//  free(neu1);
//  free(neu1e);
#endif
}

#endif //__W2V_MODEL_SKIPGRAM_HPP__
