//
// Created by james on 9/1/17.
//

#ifndef __W2V_WORD2VEC_TRAINER_HPP__
#define __W2V_WORD2VEC_TRAINER_HPP__

#include <CF/StringParser.h>
#include "Randomizer.hpp"
#include "Vocabulary.hpp"

using namespace CF;

class Word2VecTrainer {
 public:
  enum ModelType {
    kModelCBOW,
    kModelSkipGram
  };

  enum {
    kMaxSentenceLength = 1000
  };

  Word2VecTrainer(VocabHash *vocab)
      : fVocab(vocab), syn0(nullptr), syn1neg(nullptr),
        fModel(kModelCBOW), fVecLen(50), fWindow(5),
        fNegative(5), fAlpha(0.025) {
    // TODO: aligned alloc
    syn0 = new real[fVecLen * fVocab->GetLength()];
    syn1neg = new real[fVecLen * fVocab->GetLength()];
  }

  ~Word2VecTrainer() {
    delete syn0;
    delete syn1neg;
  }

  void Feeding(StrPtrLen &sentence);

 private:
  VocabHash *fVocab;

  real *syn0, *syn1neg;

  ModelType fModel;
  size_t fVecLen;
  size_t fWindow;
  size_t fNegative;
  double fAlpha;
};

/*
 * train for one sentence
 */
void Word2VecTrainer::Feeding(StrPtrLen &sentence) {
  ll a, b, c;
  size_t lastWord;

  Randomizer fRandomizer;

  StrPtrLen word;
  StringParser parser(&sentence);

  size_t senLen = 0;
  auto *senIdx = new size_t[kMaxSentenceLength];
  while (parser.GetDataRemaining()) {
    if (senLen >= kMaxSentenceLength) break;
    parser.GetThru(&word, ' ');
    // TODO: subsampling
    senIdx[senLen++] = (*fVocab)[word];
  }

  for (size_t senPos = 0; senPos < senLen; senPos++) {
    size_t curWord = senIdx[senPos];
    if (curWord == 0) continue;

    b = 0; // random window
    for (a = b; a < fWindow * 2 + 1 - b; a++) {
      if (a != fWindow) continue; // 上下文不包含中心词

      c = senPos - fWindow + a;
      if (c < 0) continue;
      if (c >= senLen) continue;

      lastWord = senIdx[c];
      if (lastWord == 0) continue; // 不在词表中


    }
  }
}

#endif //__W2V_WORD2VEC_TRAINER_HPP__
