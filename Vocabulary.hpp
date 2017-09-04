//
// Created by james on 9/1/17.
//

#ifndef __W2V_VOCABULARY_HPP__
#define __W2V_VOCABULARY_HPP__

#include <string.h>
#include <CF/StrPtrLen.h>
#include "types.h"
#include "Sampler.hpp"

using namespace CF;

class Word : public Countable {
 public:
  static ull GetWordHash(char *word) {
    ull hash = 0;
    for (size_t a = 0; a < ::strlen(word); a++)
      hash = hash * 257 + word[a];
    return hash;
  }
  static ull GetWordHash(StrPtrLen &word) {
    ull hash = 0;
    for (size_t a = 0; a < word.Len; a++)
      hash = hash * 257 + word[a];
    return hash;
  }

  Word(char *word)
      : Countable(), fWord(nullptr), fLen(0), fHash(0) {
    fLen = ::strlen(word);
    fWord = new char[fLen + 1];
    ::strcpy(fWord, word);
    fHash = GetWordHash(fWord);
  }

  ~Word() { delete[] fWord; }

  size_t GetLength() { return fLen; }

  ull GetHashCode() { return fHash; }

  char *operator*() { return fWord; }
  char operator[](size_t i) { return fWord[i]; }

 private:
  char *fWord;
  size_t fLen;
  ull fHash;
};

class Vocabulary : public SampleSet<Word> {
 public:
  enum {
    kMinVocabSize = 1000,
  };

  Vocabulary(size_t size = kMinVocabSize)
      : fWords(nullptr), fLen(0), fSize(size) {
    fWords = new Word *[fSize];
  }

  ~Vocabulary() {
    for (size_t i = 0; i < fLen; i++) {
      delete fWords[i];
    }
    delete[] fWords;
  }

  /**
   * @note Vocabulary will manager word's memory.
   */
  size_t InsertWord(Word *word) {
    // extern memory
    if (fLen == fSize) {
      auto tmp = new Word *[fSize * 2];
      ::memcpy(tmp, fWords, sizeof(Word *) * fSize);
      fSize <<= 1;
    }
    fWords[fLen] = word;
    return fLen++;
  }

  size_t InsertWord(char *word) {
    return InsertWord(new Word(word));
  }

  size_t GetLength() override { return fLen; }

  Word &operator[](size_t i) override {
    return *fWords[i];
  }

 private:
  Word **fWords;
  size_t fLen;
  size_t fSize;
};

class VocabHash {
 public:
  enum {
    kMinBucketSize = 2000,
  };

  VocabHash(size_t size = kMinBucketSize)
      : fVocab(), fHashBucket(nullptr), fBucketSize(size) {
    fVocab.InsertWord("\n");
    fHashBucket = new size_t[fBucketSize + 1];
    ::memset(fHashBucket, 0, sizeof(size_t) * (fBucketSize + 1));
  }

  ~VocabHash() { delete fHashBucket; }

  void InsertWord(char *word) {
    // extend memory
    if (fVocab.GetLength() * 1.5 >= fBucketSize) {
      auto tmp = new size_t[fBucketSize * 2 + 1];
      fHashBucket = tmp;
      fBucketSize <<= 1;
      rebuildHash();
    }

    size_t idx = Word::GetWordHash(word) % fBucketSize;
    while (fHashBucket[idx] != 0) {
      if (::strcmp(word, *fVocab[fHashBucket[idx]]) == 0) return;
      idx++;
    };
    fHashBucket[idx] = fVocab.InsertWord(word);
  }

  Vocabulary &GetVocab() { return fVocab; }

  size_t GetLength() { return fVocab.GetLength(); }

  size_t operator[](char *word) {
    size_t idx = Word::GetWordHash(word) % fBucketSize;
    while (fHashBucket[idx] != 0) {
      if (::strcmp(word, *fVocab[fHashBucket[idx]]) == 0)
        return fHashBucket[idx];
      idx++;
    };
    return 0;
  }

  size_t operator[](StrPtrLen &word) {
    size_t idx = Word::GetWordHash(word) % fBucketSize;
    while (fHashBucket[idx] != 0) {
      if (word.Equal(*fVocab[fHashBucket[idx]]))
        return fHashBucket[idx];
      idx++;
    };
    return 0;
  }

 private:
  void rebuildHash() {
    ::memset(fHashBucket, 0, sizeof(size_t) * (fBucketSize + 1));
    for (size_t i = 1; i < fVocab.GetLength(); i++) {
      size_t idx = fVocab[i].GetHashCode() % fBucketSize;
      while (fHashBucket[idx] != 0) idx++;
      fHashBucket[idx] = i;
    }
  }

  Vocabulary fVocab;
  size_t *fHashBucket; // last element must 0
  size_t fBucketSize; // equal sizeof(fHashBucket)-1
};

#endif //__W2V_VOCABULARY_HPP__
