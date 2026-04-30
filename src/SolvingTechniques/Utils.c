#include "Utils.h"

// ############################## Funkcje pomocnicze do rozwiązywania ##############################

// 1. Sprawdza czy maska ma tylko jeden zapalony bit (potęga dwójki)
int hasSingleCandidate(unsigned short mask) {
  return mask != 0 && (mask & (mask - 1)) == 0;
}

// 2. Jeśli maska ma 1 bit, zwraca która to cyfra (np. z 00100000 wyciągnie 6)
int getCandidateValue(unsigned short mask) {
  int val = 1;
  while (!(mask & 1)) {
    mask >>= 1;
    val++;
  }
  return val;
}

// 3. Sprawdza, czy w masce danej komórki zapalony jest bit dla cyfry 'val'
int isCandidatePossible(unsigned short mask, int val) {
  return (mask & (1 << (val - 1))) != 0;
}



// ################################ Metody maski ##############################

void createMask(char* sudoku, unsigned short* masks) {

  for (int i = 0; i < 81; i++) {
    if (sudoku[i] == '0') {
      masks[i] = 0x1FF;
    }
    else {
      masks[i] = 0;
    }
  }

  for (int i = 0; i < 81; i++) {
    if (sudoku[i] != '0') {
      int val = sudoku[i] - '0';

      int row = i / W;
      int col = i % W;
      int boxStartRow = (row / 3) * 3;
      int boxStartCol = (col / 3) * 3;

      unsigned short bitToClear = ~(1 << (val - 1));

      for (int j = 0; j < W; j++) {
        masks[row * W + j] &= bitToClear;
        masks[j * W + col] &= bitToClear;
      }

      for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 3; c++) {
          masks[(boxStartRow + r) * W + (boxStartCol + c)] &= bitToClear;
        }
      }
    }
  }
}

void updateMasksAfterPlacement(unsigned short* masks, int idx, int val) {
  int row = idx / W;
  int col = idx % W;
  int boxStartRow = (row / 3) * 3;
  int boxStartCol = (col / 3) * 3;

  unsigned short bitToClear = ~(1 << (val - 1));

  for (int j = 0; j < W; j++) {
    masks[row * W + j] &= bitToClear;
    masks[j * W + col] &= bitToClear;
  }

  for (int r = 0; r < 3; r++) {
    for (int c = 0; c < 3; c++) {
      masks[(boxStartRow + r) * W + (boxStartCol + c)] &= bitToClear;
    }
  }
}