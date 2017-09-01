//
// Created by james on 9/1/17.
//

#ifndef __W2V_VEC_MODEL_H__
#define __W2V_VEC_MODEL_H__

#include <stdlib.h>
#include <Vocabulary.hpp>
#include <CF/StringParser.h>

using namespace CF;

typedef double real;

class VecModel {
 public:
  enum Model {
    kModelCBOW,
    kModelSkipGram
  };

  VecModel() : fModel(kModelCBOW), fVecLen(50) {}

  void Feeding(char *sentence);

 private:
  Model fModel;
  size_t fVecLen;
  double fAlpha;
};

char *GetWord(char *sen, );

void VecModel::Feeding(char *sentence) {

  StrPtrLen word, sen(sentence);
  StringParser parser(&sen);

  while (parser.GetDataRemaining()) {
    parser.GetThru(&word, ' ');

  }

  real *neu1 = (real *) calloc(fVecLen, sizeof(real));
  real *neu1e = (real *) calloc(fVecLen, sizeof(real));

  if (fModel == kModelCBOW) {

  } else {

  }
}

#endif //__W2V_VEC_MODEL_H__
