//
// Created by James on 2017/9/3.
//

#ifndef __W2V_SAMPLER_HPP__
#define __W2V_SAMPLER_HPP__

#include "math.h"
#include "types.h"
#include "Randomizer.hpp"

template<class T>
class Sampler;

class Countable {
 public:
  Countable(size_t count = 0) : fCount(count) {}

  void Reset(size_t count = 0) { fCount = count; }

  size_t Add(size_t count = 1) { return fCount += count; }

  size_t Count() { return fCount; }

 private:
  size_t fCount;

  template<class T>
  friend class Sampler;
};

template<class T>
class SampleSet {
 public:
  virtual size_t GetLength() = 0;
  virtual T &operator[](size_t i) = 0;
};

template<class T>
class Sampler {
 public:
  enum {
    kDefaultGranularity = 100000000,
  };

  Sampler(SampleSet<T> &set, size_t granularity = kDefaultGranularity)
      : fTableSize(granularity) {
    real d1, power = 0.75;
    ll powSum = 0;
    size_t i, a;

    fTable = new size_t[fTableSize];

    fSampleSize = set.GetLength();
    for (i = 0; i < fSampleSize; i++) {
      powSum += pow(static_cast<Countable &>(set[i]).fCount, power);
    }

    i = 0;
    d1 = (real) (pow(static_cast<Countable &>(set[i]).fCount, power)
        / (real) powSum);
    for (a = 0; a < fTableSize; a++) {
      if (a / (real) fTableSize >= d1) { // 计算下一个词的概率长度
        i++;
        if (i >= fSampleSize) {
          i = fSampleSize - 1;
        } else {
          d1 += pow(static_cast<Countable &>(set[i]).fCount, power)
              / (real) powSum;
        }
      }
      fTable[a] = i;
    }
  }

  size_t Sampling() {
    size_t random = fRandomizer.Next();
    size_t target = fTable[(random >> 16) % fTableSize];
    if (target == 0) target = random % (fSampleSize - 1) + 1;
    return target;
  }

 private:
  Randomizer fRandomizer;
  size_t fTableSize, fSampleSize;
  size_t *fTable;
};

#endif //__W2V_SAMPLER_HPP__
