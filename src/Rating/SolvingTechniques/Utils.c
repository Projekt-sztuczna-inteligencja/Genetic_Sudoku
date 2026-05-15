#include "Utils.h"
#include <stdio.h>

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

// 4. Liczy, ile bitów jest zapalonych w masce (ile kandydatów pozostało)
int countCandidates(unsigned short mask) {
  int count = 0;
  while (mask) {
    mask &= (mask - 1);
    count++;
  }
  return count;
}

int validateSudoku(char* sudoku) {
  // Sprawdzenie wierszy
  for (int row = 0; row < 9; row++) {
    int seen[10] = { 0 };
    for (int col = 0; col < 9; col++) {
      int val = sudoku[row * 9 + col] - '0';
      if (val < 1 || val > 9 || seen[val]) return 0;
      seen[val] = 1;
    }
  }

  // Sprawdzenie kolumn
  for (int col = 0; col < 9; col++) {
    int seen[10] = { 0 };
    for (int row = 0; row < 9; row++) {
      int val = sudoku[row * 9 + col] - '0';
      if (val < 1 || val > 9 || seen[val]) return 0;
      seen[val] = 1;
    }
  }

  // Sprawdzenie pudełek
  for (int boxId = 0; boxId < 9; boxId++) {
    int seen[10] = { 0 };
    int boxStartRow = (boxId / 3) * 3;
    int boxStartCol = (boxId % 3) * 3;

    for (int r = 0; r < 3; r++) {
      for (int c = 0; c < 3; c++) {
        int val = sudoku[(boxStartRow + r) * 9 + (boxStartCol + c)] - '0';
        if (val < 1 || val > 9 || seen[val]) return 0;
        seen[val] = 1;
      }
    }
  }

  return 1;
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

//################################# Metody pomocnicze do rozwiązywania ##############################

int cellsSeeEachOther(int idx1, int idx2) {
  if (idx1 == idx2) return 0;
  int r1 = idx1 / 9, c1 = idx1 % 9, b1 = (r1 / 3) * 3 + (c1 / 3);
  int r2 = idx2 / 9, c2 = idx2 % 9, b2 = (r2 / 3) * 3 + (c2 / 3);
  return (r1 == r2 || c1 == c2 || b1 == b2);
}

int processNakedSubset(char* sudoku, unsigned short* masks, int* indices, int n) {
  int changed = 0;

  // Używamy zagnieżdżonych pętli do wyboru kombinacji N komórek z 9.
  // Dla Triples (n=3) i Quads (n=4) to podejście jest bardzo szybkie.
  for (int i = 0; i < 9; i++) {
    if (sudoku[indices[i]] != '0') continue;
    for (int j = i + 1; j < 9; j++) {
      if (sudoku[indices[j]] != '0') continue;
      for (int k = j + 1; k < 9; k++) {
        if (sudoku[indices[k]] != '0') continue;

        // Logika dla Triples (n=3)
        if (n == 3) {
          unsigned short combinedMask = masks[indices[i]] | masks[indices[j]] | masks[indices[k]];
          if (countCandidates(combinedMask) == 3) {
            // Znaleziono Naked Triple! Usuwamy te 3 kandydatów z reszty grupy
            for (int m = 0; m < 9; m++) {
              int idx = indices[m];
              if (idx != indices[i] && idx != indices[j] && idx != indices[k] && sudoku[idx] == '0') {
                if (masks[idx] & combinedMask) {
                  masks[idx] &= ~combinedMask;
                  changed = 1;
                }
              }
            }
          }
        }

        // Logika dla Quads (n=4)
        if (n == 4) {
          for (int l = k + 1; l < 9; l++) {
            if (sudoku[indices[l]] != '0') continue;
            unsigned short combinedMask = masks[indices[i]] | masks[indices[j]] | masks[indices[k]] | masks[indices[l]];
            if (countCandidates(combinedMask) == 4) {
              for (int m = 0; m < 9; m++) {
                int idx = indices[m];
                if (idx != indices[i] && idx != indices[j] && idx != indices[k] && idx != indices[l] && sudoku[idx] == '0') {
                  if (masks[idx] & combinedMask) {
                    masks[idx] &= ~combinedMask;
                    changed = 1;
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  return changed;
}

int processHiddenSubset(char* sudoku, unsigned short* masks, int* indices, int n) {
  int changed = 0;
  // candidatePosMasks[1-9] powie nam, w których z 9 komórek grupy może wystąpić dana cyfra
  unsigned short candidatePosMasks[10] = { 0 };

  for (int val = 1; val <= 9; val++) {
    unsigned short bitVal = (1 << (val - 1));
    for (int i = 0; i < 9; i++) {
      int idx = indices[i];
      if (sudoku[idx] == '0' && (masks[idx] & bitVal)) {
        candidatePosMasks[val] |= (1 << i); // i-ty bit oznacza i-tą komórkę w grupie
      }
    }
  }

  // Szukamy kombinacji N kandydatów
  for (int i = 1; i <= 9; i++) {
    if (candidatePosMasks[i] == 0 || countCandidates(candidatePosMasks[i]) > n) continue;
    for (int j = i + 1; j <= 9; j++) {
      if (candidatePosMasks[j] == 0 || countCandidates(candidatePosMasks[j]) > n) continue;
      for (int k = j + 1; k <= 9; k++) {
        if (candidatePosMasks[k] == 0 || countCandidates(candidatePosMasks[k]) > n) continue;

        // Suma pozycji naszych 3 kandydatów
        unsigned short combinedPos = candidatePosMasks[i] | candidatePosMasks[j] | candidatePosMasks[k];

        // Jeśli te 3 cyfry zajmują łącznie dokładnie 3 komórki...
        if (countCandidates(combinedPos) == 3) {
          // Mamy Hidden Triple! Trzeba usunąć inne cyfry z tych komórek
          unsigned short hiddenBitmask = (1 << (i - 1)) | (1 << (j - 1)) | (1 << (k - 1));

          for (int p = 0; p < 9; p++) {
            if (combinedPos & (1 << p)) { // Jeśli to jedna z naszych 3 komórek
              int idx = indices[p];
              // Czyścimy wszystko POZA naszą ukrytą trójką
              if ((masks[idx] & ~hiddenBitmask) != 0) {
                masks[idx] &= hiddenBitmask;
                changed = 1;
                if (TEST) printf(">>> Hidden Triple: W komorce %d zostawiono tylko bity (%d,%d,%d)\n", idx, i, j, k);
              }
            }
          }
          if (changed) return 1; // Wracamy po znalezieniu pierwszej zmiany
        }
      }
    }
  }
  return changed;
}
int processHiddenQuad(char* sudoku, unsigned short* masks, int* indices) {
  int changed = 0;
  unsigned short candidatePosMasks[10] = { 0 };

  // 1. Mapowanie (tak samo jak u Ciebie)
  for (int val = 1; val <= 9; val++) {
    unsigned short bitVal = (1 << (val - 1));
    for (int i = 0; i < 9; i++) {
      int idx = indices[i];
      if (sudoku[idx] == '0' && (masks[idx] & bitVal)) {
        candidatePosMasks[val] |= (1 << i);
      }
    }
  }

  // 2. Szukamy kombinacji 4 kandydatów (DODAJEMY CZWARTĄ PĘTLĘ 'l')
  for (int i = 1; i <= 9; i++) {
    if (candidatePosMasks[i] == 0 || countCandidates(candidatePosMasks[i]) > 4) continue;
    for (int j = i + 1; j <= 9; j++) {
      if (candidatePosMasks[j] == 0 || countCandidates(candidatePosMasks[j]) > 4) continue;
      for (int k = j + 1; k <= 9; k++) {
        if (candidatePosMasks[k] == 0 || countCandidates(candidatePosMasks[k]) > 4) continue;

        // --- TA NOWA PĘTLA JEST KLUCZEM ---
        for (int l = k + 1; l <= 9; l++) {
          if (candidatePosMasks[l] == 0 || countCandidates(candidatePosMasks[l]) > 4) continue;

          unsigned short combinedPos = candidatePosMasks[i] | candidatePosMasks[j] | candidatePosMasks[k] | candidatePosMasks[l];

          // Szukamy dokładnie 4 komórek
          if (countCandidates(combinedPos) == 4) {
            unsigned short hiddenBitmask = (1 << (i - 1)) | (1 << (j - 1)) | (1 << (k - 1)) | (1 << (l - 1));

            for (int p = 0; p < 9; p++) {
              if (combinedPos & (1 << p)) {
                int idx = indices[p];
                if ((masks[idx] & ~hiddenBitmask) != 0) {
                  masks[idx] &= hiddenBitmask;
                  changed = 1;
                  if (TEST) printf(">>> Hidden Quad: W komorce %d zostawiono bity (%d,%d,%d,%d)\n", idx, i, j, k, l);
                }
              }
            }
            if (changed) return 1;
          }
        }
      }
    }
  }
  return changed;
}