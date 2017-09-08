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
  static ull GetWordHash(char *word, size_t len) {
    ull hash = 0;
    for (size_t a = 0; a < len; a++)
      hash = hash * 257 + (unsigned char)word[a];
    return hash;
  }
  static ull GetWordHash(StrPtrLen &word) {
    return GetWordHash(word.Ptr, word.Len);
  }
  static ull GetWordHash(char *word) {
    return GetWordHash(word, ::strlen(word));
  }

  Word(char *word, size_t len, size_t count) : Countable(count) {
    init(word, len);
  }

  Word(StrPtrLen &word, size_t count = 0) : Countable(count) {
    init(word.Ptr, word.Len);
  }

  Word(char *word, size_t count = 0) : Countable(count) {
    init(word, ::strlen(word));
  }

  ~Word() { delete[] fWord; }

  bool Equal(char *word, size_t len) {
    if (len != fLen) return false;
    for (size_t i = 0; i < len; i++) {
      if (word[i] != fWord[i]) return false;
    }
    return true;
  }

  size_t GetLength() { return fLen; }

  ull GetHashCode() { return fHash; }

  char *operator*() { return fWord; }
  char operator[](size_t i) { return fWord[i]; }

 private:
  void init(char *word, size_t len) {
    fLen = len;
    fWord = new char[fLen + 1];
    ::strncpy(fWord, word, fLen);
    fWord[fLen] = '\0';
    fHash = GetWordHash(fWord, fLen);
  }

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
      fWords = tmp;
      fSize *= 2;
    }
    fWords[fLen] = word;
    return fLen++;
  }

  size_t InsertWord(char *word, size_t len, size_t count) {
    return InsertWord(new Word(word, len, count));
  }

  size_t InsertWord(char *word, size_t count = 0) {
    return InsertWord(new Word(word, count));
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
    kMinBucketSize = 20,
    kDefaultBucketSize = 2000,
  };

  VocabHash(size_t size = kDefaultBucketSize)
      : fVocab(), fHashBucket(nullptr),
        fBucketSize(size > kMinBucketSize ? size : kMinBucketSize) {
    fVocab.InsertWord("\n"); // index 0 of vocab is sentinel
    fHashBucket = new size_t[fBucketSize];
    ::memset(fHashBucket, 0, sizeof(size_t) * fBucketSize);
  }

  ~VocabHash() { delete fHashBucket; }

  void InsertWord(char *word, size_t len, size_t count) {
    // extend memory
    if (fVocab.GetLength() * 1.5 >= fBucketSize) {
      auto tmp = new size_t[fBucketSize * 2];
      fHashBucket = tmp;
      fBucketSize *= 2;
      rebuildHash();
    }

    size_t idx = Word::GetWordHash(word, len) % fBucketSize;
    while (fHashBucket[idx] != 0) {
      Word &w = fVocab[fHashBucket[idx]];
      if (w.Equal(word, len)) { // exist
        return (void) w.Add(count);
      }
      idx = (idx + 1) % fBucketSize;
    };
    fHashBucket[idx] = fVocab.InsertWord(word, len, count);
  }

  void InsertWord(StrPtrLen &word, size_t count = 0) {
    return InsertWord(word.Ptr, word.Len, count);
  }

  void InsertWord(char *word, size_t count = 0) {
    return InsertWord(word, ::strlen(word), count);
  }

  Vocabulary &GetVocab() { return fVocab; }

  size_t GetLength() { return fVocab.GetLength(); }

  size_t operator[](char *word) {
    size_t idx = Word::GetWordHash(word) % fBucketSize;
    while (fHashBucket[idx] != 0) {
      if (::strcmp(word, *fVocab[fHashBucket[idx]]) == 0)
        return fHashBucket[idx];
      idx = (idx + 1) % fBucketSize;
    };
    return 0;
  }

  size_t operator[](StrPtrLen &word) {
    size_t idx = Word::GetWordHash(word) % fBucketSize;
    while (fHashBucket[idx] != 0) {
      if (word.Equal(*fVocab[fHashBucket[idx]]))
        return fHashBucket[idx];
      idx = (idx + 1) % fBucketSize;
    };
    return 0;
  }

 private:
  void rebuildHash() {
    ::memset(fHashBucket, 0, sizeof(size_t) * fBucketSize);
    for (size_t i = 1; i < fVocab.GetLength(); i++) {
      size_t idx = fVocab[i].GetHashCode() % fBucketSize;
      while (fHashBucket[idx] != 0) idx = (idx + 1) % fBucketSize;
      fHashBucket[idx] = i;
    }
  }

  Vocabulary fVocab;
  size_t fBucketSize;
  size_t *fHashBucket;
};

#endif //__W2V_VOCABULARY_HPP__
