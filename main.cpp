//
// Created by james on 8/27/17.
//

#include <CF/CF.h>
#include "word2vec.h"

using namespace CF;

CF_Error DefaultCGI(CF::Net::HTTPPacket &request,
                    CF::Net::HTTPPacket &response);

class MyConfig : public CFConfigure {
 public:
  HTTPMapping *GetHttpMapping() override {
    static HTTPMapping defaultHttpMapping[] = {
        {"/exit", (CF_CGIFunction) DefaultExitCGI},
        {"/construct", (CF_CGIFunction) DefaultCGI},
        {"/destruct", (CF_CGIFunction) DefaultCGI},
        {"/literate", (CF_CGIFunction) DefaultCGI},
        {"/ready", (CF_CGIFunction) DefaultCGI},
        {"/feeding", (CF_CGIFunction) DefaultCGI},
        {NULL, NULL}
    };
    return defaultHttpMapping;
  }
};

CF_Error CFInit(int argc, char **argv) {
  CFConfigure *config = new MyConfig();
  CFEnv::Register(config);
  initialize();
  return CF_NoErr;
}

CF_Error CFExit(CF_Error exitCode) {
  release();
  return CF_NoErr;
}

StrPtrLen *GenerateBody(W2V_Error err) {
  ResizeableStringFormatter formatter;
  formatter.Put("{\"code\":");
  formatter.Put(err);
  formatter.Put(",\"message\":\"");
  formatter.Put(sCodeMessage[err]);
  formatter.Put("\"}");
  return new StrPtrLen(formatter.GetAsCString(), formatter.GetCurrentOffset());
}

CF_Error DefaultCGI(CF::Net::HTTPPacket &request,
                    CF::Net::HTTPPacket &response) {
  W2V_Error retErr = W2V_NoErr;

  StrPtrLen name(const_cast<char *>(request.GetQueryValues("trainer")));
  if (name.Len == 0) {
    retErr = W2V_BadRequest;
  } else {
    StrPtrLen *path = request.GetRequestRelativeURI();
    if (path->Equal("/feeding")) {
      StrPtrLen *sentences = request.GetAndSetBody(nullptr);
      if (sentences != nullptr) {
        // memory of sentences will be managed by trainer
        retErr = feeding(name, sentences);
      } else {
        retErr = W2V_BadRequest;
      }
    } else if (path->Equal("/literate")) {
      StrPtrLen *vocab = request.GetBody();
      if (vocab != nullptr) {
        retErr = literate(name, *vocab);
      } else {
        retErr = W2V_BadRequest;
      }
    } else if (path->Equal("/construct")) {
      retErr = construct(name);
    } else if (path->Equal("/ready")) {
      StrPtrLen type(const_cast<char *>(request.GetQueryValues("type")));
      retErr = ready(name, type);
    } else if (path->Equal("/destruct")) {
      retErr = destruct(name);
    }
  }

  StrPtrLen *respBody = GenerateBody(retErr);
  response.SetBody(respBody);

  return CF_NoErr;
}
