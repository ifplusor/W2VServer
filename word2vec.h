//
// Created by james on 9/7/17.
//

#ifndef __W2V_WORD2VEC_H__
#define __W2V_WORD2VEC_H__

#include <CF/StrPtrLen.h>

enum W2V_Error {
  W2V_NoErr = 0,
  W2V_ErrorState,
  W2V_NotExists,
  W2V_Exists,
  W2V_Busy,
  W2V_BadFormat,
  W2V_BadRequest,
  W2V_Unknown,
  W2V_Size
};

extern CF::StrPtrLen sCodeMessage[W2V_Size];

W2V_Error initialize();
W2V_Error release();

W2V_Error construct(CF::StrPtrLen &name);
W2V_Error destruct(CF::StrPtrLen &name);
W2V_Error literate(CF::StrPtrLen &name, CF::StrPtrLen &vocab);
W2V_Error ready(CF::StrPtrLen &name, CF::StrPtrLen &type);
W2V_Error feeding(CF::StrPtrLen &name, CF::StrPtrLen *sentences);

#endif // __W2V_WORD2VEC_H__
