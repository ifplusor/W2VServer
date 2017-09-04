//
// Created by james on 8/27/17.
//

#include <CF/CF.h>
#include "Vocabulary.hpp"
#include "Word2VecTrainer.hpp"

using namespace CF;

CF_Error DefaultCGI(CF::Net::HTTPPacket &request,
                    CF::Net::HTTPPacket &response);

class MyConfig : public CFConfigure {
 public:
  HTTPMapping *GetHttpMapping() override {
    static HTTPMapping defaultHttpMapping[] = {
        {"/exit", (CF_CGIFunction) DefaultExitCGI},
        {"/", (CF_CGIFunction) DefaultCGI},
        {NULL, NULL}
    };
    return defaultHttpMapping;
  }
};

RefTable *sTrainerTable = nullptr;

CF_Error CFInit(int argc, char **argv) {
  CFConfigure *config = new MyConfig();
  CFEnv::Register(config);
  sTrainerTable = new RefTable();
  return CF_NoErr;
}

CF_Error CFExit(CF_Error exitCode) {
  delete sTrainerTable;
  return CF_NoErr;
}

/*
 * 创建训练器
 */
void construct(StrPtrLen *name) {

  Ref *ref = sTrainerTable->Resolve(name);
  if (ref != nullptr) {
    sTrainerTable->Release(ref);
    return;
  }

  Word2VecTrainer *trainer = new Word2VecTrainer(name->GetAsCString());
  sTrainerTable->Register(trainer->GetRef());
}

/*
 * 销毁训练器
 */
void destruct(StrPtrLen *name) {
  Ref *ref = sTrainerTable->Resolve(name);
  if (ref != nullptr) {
    sTrainerTable->UnRegister(ref, 1);
    delete ref->GetObject();
  }
}

void literate(StrPtrLen *name) {
  Ref *ref = sTrainerTable->Resolve(name);
  if (ref != nullptr) {
    Word2VecTrainer *trainer = static_cast<Word2VecTrainer *>(ref->GetObject());
    trainer->AddVocab();
  }
}

/*
 * 用句子训练模型
 */
void feeding(StrPtrLen *name, StrPtrLen &sentence) {
  Ref *ref = sTrainerTable->Resolve(name);
  if (ref != nullptr) {
    Word2VecTrainer *trainer = static_cast<Word2VecTrainer *>(ref->GetObject());
    trainer->Feeding(sentence);
  }
}

CF_Error DefaultCGI(CF::Net::HTTPPacket &request,
                    CF::Net::HTTPPacket &response) {
  ResizeableStringFormatter formatter(nullptr, 0);
  formatter.Put("test content\n");
  StrPtrLen *content = new StrPtrLen(formatter.GetAsCString(),
                                     formatter.GetCurrentOffset());
  response.SetBody(content);

  construct(nullptr);

  return CF_NoErr;
}
