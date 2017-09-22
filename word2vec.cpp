//
// Created by james on 9/7/17.
//

#include "word2vec.h"

#include <CF/Core/Mutex.h>
#include <CF/Ref.h>
#include "Word2VecTrainer.hpp"

using namespace CF;

StrPtrLen sCodeMessage[W2V_Size] = {
    StrPtrLen("ok", 2),
    StrPtrLen("error state", 12),
    StrPtrLen("not exist", 9),
    StrPtrLen("exist", 5),
    StrPtrLen("busy", 4),
    StrPtrLen("bad format", 10),
    StrPtrLen("bad request", 11),
    StrPtrLen("unknown", 7),
};

RefTable *sTrainerTable = nullptr;

W2V_Error initialize() {
  sTrainerTable = new RefTable();
  Word2VecTrainTask::Initial();
  return W2V_NoErr;
}

W2V_Error release() {
  delete sTrainerTable;
  return W2V_NoErr;
}

/**
 * @brief 创建训练器
 * @note synchronized
 */
W2V_Error construct(StrPtrLen &name) {
  struct Core::Mutex sMutex;
  Core::MutexLocker locker(&sMutex);

  W2V_Error err = W2V_NoErr;
  Ref *ref = sTrainerTable->Resolve(&name);
  if (ref == nullptr) {
    auto *trainer = new Word2VecTrainer(name.GetAsCString());
    sTrainerTable->Register(trainer->GetRef());
  } else {
    err = W2V_Exists;
  }
  return err;
}

/**
 * @brief 销毁训练器
 */
W2V_Error destruct(StrPtrLen &name) {
  W2V_Error err = W2V_NoErr;
  Ref *ref = sTrainerTable->Resolve(&name);
  if (ref != nullptr) {
    auto *trainer = static_cast<Word2VecTrainer *>(ref->GetObject());
    if (trainer->CanDestruct() && sTrainerTable->TryUnRegister(ref, 1)) {
        delete (Word2VecTrainer *)ref->GetObject();
    } else {
      sTrainerTable->Release(ref);
      err = W2V_Busy;
    }
  } else {
    err = W2V_NotExists;
  }
  return err;
}

/**
 * @brief 加载词典
 */
W2V_Error literate(StrPtrLen &name, StrPtrLen &vocab) {
  W2V_Error err = W2V_NoErr;
  Ref *ref = sTrainerTable->Resolve(&name);
  if (ref != nullptr) {
    auto *trainer = static_cast<Word2VecTrainer *>(ref->GetObject());
    if (trainer->CanAddVocab()) {
      StringParser checkParser(&vocab);
      while (checkParser.GetDataRemaining() > 0) {
        if (!checkParser.GetThru(nullptr, '\t') ||
            checkParser.ConsumeInteger() == 0 ||
            !checkParser.ExpectEOL()) {
          err = W2V_BadFormat;
          break;
        }
      }
      if (err == W2V_NoErr) {
        StrPtrLen word;
        UInt32 count;
        StringParser vocabParser(&vocab);
        while (vocabParser.GetDataRemaining() > 0) {
          vocabParser.GetThru(&word, '\t');
          count = vocabParser.ConsumeInteger();
          vocabParser.ConsumeEOL(nullptr);
          trainer->AddWordToVocab(word, count);
        }
      }
    } else {
      err = W2V_ErrorState;
    }
    sTrainerTable->Release(ref);
  } else {
    err = W2V_NotExists;
  }
  return err;
}

/**
 * @brief 训练就绪
 */
W2V_Error ready(StrPtrLen &name, StrPtrLen &type) {
  W2V_Error err = W2V_NoErr;
  Ref *ref = sTrainerTable->Resolve(&name);
  if (ref != nullptr) {
    auto *trainer = static_cast<Word2VecTrainer *>(ref->GetObject());
    if (type.Equal("cbow")) {
      trainer->InitModel(Word2VecTrainer::kModelCBOW);
    } else {
      trainer->InitModel(Word2VecTrainer::kModelSkipGram);
    }
    sTrainerTable->Release(ref);
  } else {
    err = W2V_NotExists;
  }
  return err;
}

/**
 * @brief 用句子训练模型，非阻塞
 */
W2V_Error feeding(StrPtrLen &name, StrPtrLen *sentences) {
  W2V_Error err = W2V_NoErr;
  Ref *ref = sTrainerTable->Resolve(&name);
  if (ref != nullptr) {
    auto *trainer = static_cast<Word2VecTrainer *>(ref->GetObject());
    if (trainer->CanFeeding()) {
      trainer->Feeding(sentences);
    } else {
      err = W2V_ErrorState;
    }
    sTrainerTable->Release(ref);
  } else {
    err = W2V_NotExists;
  }
  if (err != W2V_NoErr)
    delete sentences;
  return err;
}

W2V_Error dump(StrPtrLen &name) {
  W2V_Error err = W2V_NoErr;
  Ref *ref = sTrainerTable->Resolve(&name);
  if (ref != nullptr) {
    auto *trainer = static_cast<Word2VecTrainer *>(ref->GetObject());
    sTrainerTable->Release(ref);
  } else {
    err = W2V_NotExists;
  }
  return err;
}
