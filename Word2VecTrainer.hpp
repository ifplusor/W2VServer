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

  /**
   * @note memory of name will be manager by Trainer
   */
  Word2VecTrainer(char *name, size_t vecLen = 50, size_t window = 5,
                  size_t negative = 5, double alpha = 0.025)
      : fName(name), fRef(), fState(kStateNoVocab), fVocab(nullptr),
        fModel(nullptr), fVecLen(vecLen), fWindow(window), fNegative(negative),
        fAlpha(alpha) {
    fRef.Set(fName, this);
  }

  ~Word2VecTrainer() {
    delete fName.Ptr;
    delete fModel;
    delete fVocab;
  }

  bool CanDestruct() {
    return true;
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

  bool CanFeeding() {
    return fState == kStateAlready;
  }

  /**
 * @brief train model with sentences
 *
 * push sentence to corpus queue, than signal train threads.
 */
  bool Feeding(StrPtrLen *sentences) {
    if (sentences != nullptr) {
      Word2VecTrainTask *task = new Word2VecTrainTask(this, sentences);
      task->SetThreadPicker(Thread::Task::GetBlockingTaskThreadPicker());
      task->Signal(Thread::Task::kUpdateEvent);
    }
    return true;
  }

  bool Dump() {}

  Ref *GetRef() { return &fRef; }

 private:
  TrainerState fState;

  VocabHash *fVocab;
  Word2VecModel *fModel;

  size_t fVecLen;   // vector length
  size_t fWindow;   // window size
  size_t fNegative; // sampling number
  double fAlpha;    // init alpha

  StrPtrLen fName;
  Ref fRef;

  friend class Word2VecTrainTask;
};

/**
 * @note once task
 */
class Word2VecTrainTask : public Thread::Task {
 public:
  enum {
    kMaxSentenceLength = 1000
  };

  static void Initial() {
#if __PTHREADS__
    pthread_key_create(&gSentenceKey, [](void *v) {
      delete[] v;
    });
#endif
  }

  Word2VecTrainTask(Word2VecTrainer *trainer, StrPtrLen *corpus)
      : Task(), fCorpus(corpus), fWindow(trainer->fWindow),
        fVocab(trainer->fVocab), fModel(trainer->fModel),
        fRandomizer() {
    this->SetTaskName("Word2VecTrainTask");
  }

  ~Word2VecTrainTask() override {
    delete fCorpus;
  }

  SInt64 Run() override;

 private:
  Randomizer fRandomizer;

  VocabHash *fVocab;
  Word2VecModel *fModel;
  size_t fWindow;   // window size

  StrPtrLen *fCorpus;

#if __PTHREADS__
  static pthread_key_t gSentenceKey = 0;
#endif

  friend class Word2VecTrainer;
};

SInt64 Word2VecTrainTask::Run() {
  EventFlags events = GetEvents();
  if (!(events & kUpdateEvent)) return -1;

  size_t a, b, c, lastWord, senLen;
  StrPtrLen sentence, word;

#if __PTHREADS__
  size_t *senIdx = static_cast<size_t *>(pthread_getspecific(gSentenceKey));
  if (senIdx == nullptr) {
    senIdx = new size_t[kMaxSentenceLength];
    pthread_setspecific(gSentenceKey, senIdx);
  }
#else
  size_t senIdx[kMaxSentenceLength];
#endif

  // split sentences
  size_t *ctx = new size_t[fWindow * 2];

  StringParser senParser(fCorpus);
  while (senParser.GetDataRemaining() > 0) {
    senParser.GetThruEOL(&sentence);

    // split words
    StringParser parser(&sentence);

    senLen = 0;
    while (parser.GetDataRemaining()) {
      if (senLen >= kMaxSentenceLength) break;
      parser.GetThru(&word, ' ');
      size_t idx = (*fVocab)[word];
//        if (idx == 0) continue;
      // TODO: subsampling
      senIdx[senLen++] = idx;
    }

    size_t ctxLen = 0;

    for (size_t senPos = 0; senPos < senLen; senPos++) {
      size_t cen = senIdx[senPos];
      if (cen == 0) continue;

      b = fRandomizer.Next() % fWindow; // random window
      for (a = b; a < fWindow * 2 + 1 - b; a++) {
        if (a == fWindow) continue; /* 上下文不包含中心词 */

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

  delete ctx;
  delete fCorpus; // delete corpus, it type is StrPtrLenDel

  return -1;
}

#endif //__W2V_WORD2VEC_TRAINER_HPP__
