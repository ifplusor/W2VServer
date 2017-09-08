//
// Created by james on 9/1/17.
//

#ifndef __W2V_WORD2VEC_TRAINER_HPP__
#define __W2V_WORD2VEC_TRAINER_HPP__

#include <CF/StringParser.h>
#include <CF/Thread/Task.h>
#include "Randomizer.hpp"
#include "Vocabulary.hpp"
#include "Word2VecModel.hpp"
#include "CBOWModel.hpp"
#include "SkipGramModel.hpp"

using namespace CF;

class Word2VecTrainer : public Thread::Task {
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
        fAlpha(alpha), fRandomizer(), fCorpus() {
    fRef.Set(fName, this);
  }

  ~Word2VecTrainer() {
    delete fName.Ptr;
    delete fModel;
    delete fVocab;
  }

  bool CanAddVocab() {
    if (fState == kStateNoVocab) {
      fVocab = new VocabHash();
      fState = kStateHaveVocab;
    }
    return fState == kStateHaveVocab;
  }

  bool AddWordToVocab(StrPtrLen &word, size_t count) {
    fVocab->InsertWord(word, count);
    return true;
  }

  bool CanFeeding() {
    return fState == kStateAlready;
  }

  bool InitModel(ModelType type) {
    if (fState != kStateHaveVocab) return false;

    switch (type) {
      case kModelCBOW:
        fModel = new CBOWModel(new Sampler<Word>(fVocab->GetVocab()),
                               fVocab->GetLength(),
                               fVecLen,
                               fNegative,
                               fAlpha);
        break;
      case kModelSkipGram:
        fModel = new SkipGramModel(new Sampler<Word>(fVocab->GetVocab()),
                                   fVocab->GetLength(),
                                   fVecLen,
                                   fNegative,
                                   fAlpha);
        break;
    }
    fState = kStateAlready;

    return true;
  }

  bool Feeding(StrPtrLen *sentences);

  bool Dump();

  SInt64 Run() override;

  Ref *GetRef() { return &fRef; }

 private:
  TrainerState fState;
  Randomizer fRandomizer;

  ConcurrentQueue fCorpus;

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
bool Word2VecTrainer::Feeding(StrPtrLen *sentences) {

  if (sentences != nullptr) {
    fCorpus.EnQueue(new QueueElem(&sentences));
    Signal(Thread::Task::kReadEvent);
  }

  return true;
}

bool Word2VecTrainer::Dump() {

}

SInt64 Word2VecTrainer::Run() {
  EventFlags events = GetEvents();
  if (!(events & kReadEvent)) return 0;

  size_t a, b, c, lastWord, senLen;
  size_t senIdx[kMaxSentenceLength];
  StrPtrLen sentence, word;

  for (QueueElem *elem = fCorpus.DeQueue();
       elem != nullptr;
       elem = fCorpus.DeQueue()) {
    auto *corpus = static_cast<StrPtrLen *>(elem->GetEnclosingObject());
    delete elem;

    // split sentences
    StringParser senParser(corpus);
    while (senParser.GetDataRemaining() > 0) {
      senParser.GetThruEOL(&sentence);

      // split words
      StringParser parser(&sentence);

      senLen = 0;
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
          if (c >= senLen) // since underflow, c also bigger than senLen(if c<0)
            continue;

          lastWord = senIdx[c];
          if (lastWord == 0) continue; /* 不在词表中 */

          ctx[ctxLen++] = lastWord;
        }

        fModel->Step(cen, ctx, ctxLen); // once bp
      }
    }
  }

  return 0;
}

#endif //__W2V_WORD2VEC_TRAINER_HPP__
