//
// Created by James on 2017/9/2.
//

#ifndef __W2V_RANDOMIZER_HPP__
#define __W2V_RANDOMIZER_HPP__

#include "types.h"

class Randomizer {
 public:
  Randomizer(ull seed = 0) : fNext(seed) {}

  ull Next() { return fNext = fNext * t  + 11; }

 private:
  ull fNext;

  static ull t;
};

ull Randomizer::t = 25214903917UL;

#endif //__W2V_RANDOMIZER_HPP__
