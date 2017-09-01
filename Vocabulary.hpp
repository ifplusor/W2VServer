//
// Created by james on 9/1/17.
//

#ifndef __W2V_VOCABULARY_H__
#define __W2V_VOCABULARY_H__

#include <string.h>

typedef unsigned long long ull;

class Word {
 public:
  static ull GetWordHash(char *word) {
    ull hash = 0;
    for (size_t a = 0; a < ::strlen(word); a++)
      hash = hash * 257 + word[a];
    return hash;
  }

  Word(char *word) : fWord(nullptr), fLen(0), fCount(0), fHash(0) {
    fLen = ::strlen(word);
    fWord = new char[fLen + 1];
    ::strcpy(fWord, word);
    fHash = GetWordHash(fWord);
  }

  ~Word() { delete[] fWord; }

  size_t GetLength() { return fLen; }

  ull GetHashCode() { return fHash; }

  char* operator*() { return fWord; }

 private:
  char *fWord;
  size_t fLen;
  size_t fCount;
  ull fHash;
};

class Vocabulary {
 public:
  enum {
    kMinVocabSize = 1000
  };

  Vocabulary() : fWords(nullptr), fLen(0), fSize(kMinVocabSize) {
    fWords = new Word *[fSize];
  }

  ~Vocabulary() {
    for (size_t i = 0; i < fLen; i++) {
      delete fWords[i];
    }
    delete[] fWords;
  }

  void InsertWord(char *word) {
    // extern memory
    if (fLen == fSize) {
      Word **tmp = new Word*[fSize * 2];
      ::memcpy(tmp, fWords, sizeof(Word*) * fSize);
      fSize <<= 1;
    }
    Word *w = new Word(word);
    fWords[fLen++] = w;
  }

  Word* operator[](size_t index) {
    if (index < fLen)
      return fWords[index];
    return nullptr;
  }

 private:
  Word **fWords;
  size_t fLen;
  size_t fSize;
};

#endif //__W2V_VOCABULARY_H__
