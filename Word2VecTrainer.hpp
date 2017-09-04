//
// Created by james on 9/1/17.
//

#ifndef __W2V_WORD2VEC_TRAINER_HPP__
#define __W2V_WORD2VEC_TRAINER_HPP__

#include <CF/StringParser.h>
#include "Randomizer.hpp"
#include "Vocabulary.hpp"
#include "Word2VecModel.hpp"
#include "CBOWModel.hpp"
#include "SkipGramModel.hpp"

using namespace CF;

class Word2VecTrainer {
 public:
  enum ModelType {
    kModelCBOW,
    kModelSkipGram
  };

  enum TrainerState {
    kStateNoVocab,
    kStateHaveVocab,
    kStateAlready
  };

  enum {
    kMaxSentenceLength = 1000
  };

  /**
   * @note memory of name will be manager by Trainer
   */
  Word2VecTrainer(char *name, size_t vecLen = 50, size_t window = 5,
                  size_t negative = 5, double alpha = 0.025)
      : fName(name), fRef(), fState(kStateNoVocab), fVocab(nullptr),
        fVecLen(vecLen), fWindow(window), fNegative(negative),
        fAlpha(0.alpha), fRandomizer() {
    fRef.Set(fName, this);
  }

  ~Word2VecTrainer() {
    delete fName.Ptr;
    delete fModel;
    delete fVocab;
  }

  bool AddWordToVocab(char *word) {
    if (fState == kStateNoVocab) {
      fVocab = new VocabHash();
      fState = kStateHaveVocab;
    }
    if (fState != kStateHaveVocab) return false;
    // TODO: add word to vocabulary
    fVocab->InsertWord(word);
    return true;
  }

  bool InitModel(ModelType type) {
    if (fState != kStateHaveVocab) return false;

    switch (type) {
      case kModelCBOW:
        fModel = new CBOWModel(new Sampler(fVocab->GetVocab()),
                               fVocab->GetLength(),
                               fVecLen,
                               fNegative,
                               fAlpha);
        break;
      case kModelSkipGram:
        fModel = new SkipGramModel(new Sampler(fVocab->GetVocab()),
                                   fVocab->GetLength(),
                                   fVecLen,
                                   fNegative,
                                   fAlpha);
        break;
    }
    fState = kStateAlready;

    return true;
  }

  bool Feeding(StrPtrLen &sentence);

  Ref *GetRef() { return &fRef; }

 private:
  TrainerState fState;
  Randomizer fRandomizer;

  VocabHash *fVocab;
  Word2VecModel *fModel;

  size_t fVecLen;   // vector length
  size_t fWindow;   // window size
  size_t fNegative; // sampling number
  double fAlpha;    // init alpha

  StrPtrLen fName;
  Ref fRef;
};

/**
 * @brief train for one sentence
 * @todo this function will be reenter
 */
bool Word2VecTrainer::Feeding(StrPtrLen &sentence) {
  if (fState != kStateAlready)
    return false;

  size_t a, b, c, lastWord;

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

  size_t *ctx = new size_t[fWindow * 2];
  size_t ctxLen = 0;

  for (size_t senPos = 0; senPos < senLen; senPos++) {
    size_t cen = senIdx[senPos];
    if (cen == 0) continue;

    b = fRandomizer.Next() % fWindow; // random window
    for (a = b; a < fWindow * 2 + 1 - b; a++) {
      if (a != fWindow) continue; /* 上下文不包含中心词 */

      c = senPos + a - fWindow;
      if (c >= senLen) // since underflow, c also bigger than senLen when c<0
        continue;

      lastWord = senIdx[c];
      if (lastWord == 0) continue; /* 不在词表中 */

      ctx[ctxLen++] = lastWord;
    }

    fModel->Step(cen, ctx, ctxLen); // once bp
  }

  return true;
}

#endif //__W2V_WORD2VEC_TRAINER_HPP__
