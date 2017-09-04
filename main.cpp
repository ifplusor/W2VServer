//
// Created by james on 8/27/17.
//

#include <CF/CF.h>
#include "Vocabulary.hpp"
#include "Word2VecTrainer.hpp"

using namespace CF;

void test(StrPtrLen *name);

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

 private:
  static CF_Error DefaultCGI(CF::Net::HTTPPacket &request,
                             CF::Net::HTTPPacket &response) {
    ResizeableStringFormatter formatter(nullptr, 0);
    formatter.Put("test content\n");
    StrPtrLen *content = new StrPtrLen(formatter.GetAsCString(),
                                       formatter.GetCurrentOffset());
    response.SetBody(content);

    test(nullptr);

    return CF_NoErr;
  }
};

CF_Error CFInit(int argc, char **argv) {
  CFConfigure *config = new MyConfig();
  CFEnv::Register(config);
  return CF_NoErr;
}

CF_Error CFExit(CF_Error exitCode) {
  return CF_NoErr;
}

void test(StrPtrLen *name) {
  Vocabulary *vocab = new Vocabulary();
  Sampler<Word> *sampler = new Sampler<Word>(*vocab);

  VocabHash *vocabHash = new VocabHash();

  Word2VecTrainer *trainer = new Word2VecTrainer(vocabHash);

  delete sampler;
  delete vocab;
}