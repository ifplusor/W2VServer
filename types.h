//
// Created by James on 2017/9/2.
//

#ifndef __W2V_TYPES_H__
#define __W2V_TYPES_H__

#include <stdlib.h>

enum {
  kMaxVectorLength = 2000,
  kMaxSentenceLength = 1000,
  kMaxContextLength = 20,
};

typedef long long ll;
typedef unsigned long long ull;


#if USE_FLOAT_AS_REAL

typedef float real;

#define blas_copy cblas_scopy
#define blas_dot cblas_sdot
#define blas_axpy cblas_saxpy

#else

typedef double real;

#define blas_copy cblas_dcopy
#define blas_dot cblas_ddot
#define blas_axpy cblas_daxpy

#endif

#endif //__W2V_TYPES_H__
