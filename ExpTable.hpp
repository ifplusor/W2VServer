//
// Created by James on 2017/9/2.
//

#ifndef __W2V_EXP__TABLE_HPP__
#define __W2V_EXP__TABLE_HPP__

#include <math.h>
#include "types.h"

class ExpTable {
 public:
  enum {
    kMaxExp = 6,
    kDefaultExpTableSize = 1000
  };

  ExpTable(size_t granularity = kDefaultExpTableSize)
      : fExpTableSize(granularity) {
    fExpTable = new real[fExpTableSize + 1];
    for (size_t i = 0; i <= fExpTableSize; i++) {
      // pre-compute the exp() table
      fExpTable[i] = (real) exp((i / (real) fExpTableSize * 2 - 1) * kMaxExp);
      // pre-compute f(x) = x / (x + 1)
      fExpTable[i] = fExpTable[i] / (fExpTable[i] + 1);
    }
  }

  ~ExpTable() {
    delete fExpTable;
  }

  inline real Sigmoid(real x) {
    if (x > kMaxExp) return 1;
    if (x < -kMaxExp) return 0;
    return fExpTable[(int) ((x + kMaxExp) * (fExpTableSize / kMaxExp / 2))];
  }

 private:
  real *fExpTable;
  size_t fExpTableSize;
};

static ExpTable *expTable = new ExpTable();

inline real sigmoid(real x) {
  return expTable->Sigmoid(x);
}

#endif //__W2V_EXP__TABLE_HPP__
