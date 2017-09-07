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

  Ref *ref = sTrainerTable->Resolve(&name);
  if (ref != nullptr) {
    sTrainerTable->Release(ref);
    return W2V_Exists;
  }

  auto *trainer = new Word2VecTrainer(name.GetAsCString());
  sTrainerTable->Register(trainer->GetRef());

  return W2V_NoErr;
}

/*
 * 销毁训练器
 */
W2V_Error destruct(StrPtrLen &name) {
  Ref *ref = sTrainerTable->Resolve(&name);
  if (ref != nullptr) {
    if (!sTrainerTable->TryUnRegister(ref, 1)) {
      sTrainerTable->Release(ref);
      return W2V_Busy;
    }
    delete ref->GetObject();
  } else {
    return W2V_NotExists;
  }
  return W2V_NoErr;
}

/*
 * 加载词典
 */
W2V_Error literate(StrPtrLen &name, StrPtrLen &vocab) {
  Ref *ref = sTrainerTable->Resolve(&name);
  if (ref != nullptr) {
    auto *trainer = static_cast<Word2VecTrainer *>(ref->GetObject());
    if (!trainer->CanAddVocab()) return W2V_ErrorState;
    StringParser checkParser(&vocab);
    while (checkParser.GetDataRemaining() > 0) {
      if (!checkParser.GetThru(nullptr, '\t')) return W2V_BadFormat;
      if (checkParser.ConsumeInteger() == 0) return W2V_BadFormat;
      if (!checkParser.ExpectEOL()) return W2V_BadFormat;
    }
    StrPtrLen word;
    UInt32 count;
    StringParser vocabParser(&vocab);
    while (vocabParser.GetDataRemaining() > 0) {
      vocabParser.GetThru(&word, '\t');
      count = vocabParser.ConsumeInteger();
      vocabParser.ConsumeEOL(nullptr);
      trainer->AddWordToVocab(word, count);
    }
    sTrainerTable->Release(ref);
  } else {
    return W2V_NotExists;
  }
  return W2V_NoErr;
}

/*
 * 训练就绪
 */
W2V_Error ready(StrPtrLen &name, StrPtrLen &type) {
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
    return W2V_NotExists;
  }
  return W2V_NoErr;
}

/*
 * 用句子训练模型
 */
W2V_Error feeding(StrPtrLen &name, StrPtrLen &sentences) {
  Ref *ref = sTrainerTable->Resolve(&name);
  if (ref != nullptr) {
    auto *trainer = static_cast<Word2VecTrainer *>(ref->GetObject());
    trainer->Feeding(sentences);
    sTrainerTable->Release(ref);
  } else {
    return W2V_NotExists;
  }
  return W2V_NoErr;
}
