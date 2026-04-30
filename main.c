#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define W 9
#define H 9
#define SW 3
#define BXS 9

#define true 1
#define false 0

#define TEST 0 // 1 - test, 0 - production

char sudokus[2][82] = {
  "042005006197000040560400109801300260900071450003256000005032700004590600000760080",
  "000102000060000070008000900400000003050007000200080001009000805070000060000304000" };

char candidates[] = {
  '1', '2', '3', '4', '5', '6', '7', '8', '9'
};

float dificulty(char* sudoku) {

}

int contains(char* tab, int size, char value) {
  for (int i = 0; i < size; i++) {
    if (tab[i] == value) {
      return 1;
    }
  }
  return 0;
}
void testi(int* box, int size) {
  if (!TEST) return;
  for (int i = 0; i < size; i++) {
    printf("%d ", box[i]);
  }
  printf("\n");
}
void test(char* box, int size) {
  if (!TEST) return;
  for (int i = 0; i < size; i++) {
    printf("%c ", box[i]);
  }
  printf("\n");
}

void getRow(char* sudoku, int rowId, char** row) {
  *row = (char*)malloc(sizeof(char) * W);
  for (int i = 0; i < W; i++) {
    (*row)[i] = sudoku[rowId * W + i];
  }
}

void getCol(char* sudoku, int colId, char** col) {
  *col = (char*)malloc(sizeof(char) * H);
  for (int i = 0; i < H; i++) {
    (*col)[i] = sudoku[i * W + colId];
  }
}

void getBox(char* sudoku, int boxId, char** box) {
  *box = (char*)malloc(sizeof(char) * W);
  for (int i = 0; i < W; i++) {
    int row = (boxId / 3) * 3 + i / 3;
    int col = (boxId % 3) * 3 + i % 3;
    (*box)[i] = sudoku[row * W + col];
  }
}

int missingInRowCol(char* sudoku) {
  // 1. SKANOWANIE WIERSZY
  for (int i = 0; i < W; i++) {
    char* row;
    getRow(sudoku, i, &row);

    // Sprawdzamy każdego możliwego kandydata (od '1' do '9')
    for (int c = 0; c < W; c++) {
      char candidate = candidates[c];

      // Jeśli cyfra już jest w tym wierszu, pomijamy ją
      if (contains(row, W, candidate)) continue;

      int possibleCount = 0;
      int lastPossibleCol = -1; // Zapamiętuje indeks ostatniego możliwego miejsca

      // Szukamy pustego miejsca w tym wierszu
      for (int j = 0; j < W; j++) {
        if (row[j] != '0') continue; // Jeśli miejsce jest zajęte, pomijamy

        // Sprawdzamy przecinającą się kolumnę
        char* col;
        getCol(sudoku, j, &col);
        int inCol = contains(col, H, candidate);
        free(col); // Pamiętamy o zwalnianiu pamięci w pętli!

        // Sprawdzamy przecinające się pudełko (box)
        int boxId = (i / 3) * 3 + (j / 3);
        char* box;
        getBox(sudoku, boxId, &box);
        int inBox = contains(box, W, candidate);
        free(box);

        // Jeśli kandydata nie ma w kolumnie ani w pudełku, to miejsce jest dozwolone
        if (!inCol && !inBox) {
          possibleCount++;
          lastPossibleCol = j;
        }
      }

      // Jeśli znaleźliśmy dokładnie JEDNO możliwe miejsce w wierszu, wstawiamy cyfrę
      if (possibleCount == 1) {
        sudoku[i * W + lastPossibleCol] = candidate;
        if (TEST) printf(">>> ZNALAZLEM (wiersze)! Wstaw %c na pozycje row: %d, col: %d\n", candidate, i, lastPossibleCol);
        free(row);
        return true;
      }
    }
    free(row);
  }

  // 2. SKANOWANIE KOLUMN
  for (int j = 0; j < W; j++) {
    char* col;
    getCol(sudoku, j, &col);

    for (int c = 0; c < W; c++) {
      char candidate = candidates[c];

      if (contains(col, H, candidate)) continue;

      int possibleCount = 0;
      int lastPossibleRow = -1;

      for (int i = 0; i < W; i++) {
        if (col[i] != '0') continue;

        // Sprawdzamy przecinający się wiersz
        char* row;
        getRow(sudoku, i, &row);
        int inRow = contains(row, W, candidate);
        free(row);

        // Sprawdzamy przecinające się pudełko
        int boxId = (i / 3) * 3 + (j / 3);
        char* box;
        getBox(sudoku, boxId, &box);
        int inBox = contains(box, W, candidate);
        free(box);

        if (!inRow && !inBox) {
          possibleCount++;
          lastPossibleRow = i;
        }
      }

      // Jeśli znaleźliśmy dokładnie JEDNO możliwe miejsce w kolumnie
      if (possibleCount == 1) {
        sudoku[lastPossibleRow * W + j] = candidate;
        if (TEST) printf(">>> ZNALAZLEM (kolumny)! Wstaw %c na pozycje row: %d, col: %d\n", candidate, lastPossibleRow, j);
        free(col);
        return true;
      }
    }
    free(col);
  }

  return false;
}

int singleCandidate(char* sudoku) {
  for (int boxId = 0; boxId < BXS; boxId++) {
    char* box;
    getBox(sudoku, boxId, &box);
    int candidatesCount = W;
    int possibleInBox[W];
    for (int i = 0; i < W; i++) {
      if (contains(box, W, candidates[i])) {
        possibleInBox[i] = false;
        candidatesCount--;
      }
      else possibleInBox[i] = true;
    }

    //iterate over box cells

    for (int cellIdx = 0; cellIdx < BXS; cellIdx++) {
      if (box[cellIdx] != '0') continue; // if cell is not empty, skip

      int rowId = (boxId / 3) * 3 + cellIdx / 3;
      int colId = (boxId % 3) * 3 + cellIdx % 3;

      char* row;
      char* col;
      getRow(sudoku, rowId, &row);
      getCol(sudoku, colId, &col);
      int possibleInCell[9];
      int candidatesCountCell = candidatesCount;


      for (int i = 0; i < W; i++) {
        possibleInCell[i] = possibleInBox[i];
        if ((possibleInCell[i]) && (contains(row, W, candidates[i]) || contains(col, W, candidates[i]))) {
          possibleInCell[i] = false;
          candidatesCountCell--;
        }

      }

      if (candidatesCountCell == 1) {
        for (int i = 0; i < W; i++) {
          if (possibleInCell[i]) {
            sudoku[rowId * W + colId] = candidates[i];
            //printf(">>> ZNALAZLEM! Wstaw %c do pudelka %d na pozycje %d\n", candidates[i], boxId, cellIdx, rowId * W + colId);
            free(row);
            free(col);
            free(box);
            return true;
          }
        }
      }
      free(row);
      free(col);
    }
    free(box);
  }
  return false;
}

// Zwraca 1 (true), jeśli w danym pudełku kandydat MOŻE wystąpić TYLKO w rzędzie targetRow
int isLockedToRow(char* sudoku, int boxId, int targetRow, char candidate) {
  char* box;
  getBox(sudoku, boxId, &box);
  // Jeśli kandydat już jest w tym pudełku, nie ma tematu
  if (contains(box, W, candidate)) {
    free(box);
    return false;
  }

  int possibleCount = 0;
  int possibleInTargetRow = 0;

  for (int i = 0; i < W; i++) {
    if (box[i] != '0') continue;
    int r = (boxId / 3) * 3 + i / 3;
    int c = (boxId % 3) * 3 + i % 3;

    char* rowStr; getRow(sudoku, r, &rowStr);
    char* colStr; getCol(sudoku, c, &colStr);

    if (!contains(rowStr, W, candidate) && !contains(colStr, W, candidate)) {
      possibleCount++;
      if (r == targetRow) {
        possibleInTargetRow++;
      }
    }
    free(rowStr); free(colStr);
  }
  free(box);

  // Jeśli są jakieś możliwe miejsca i WSZYSTKIE leżą w targetRow -> zablokowane!
  return (possibleCount > 0 && possibleCount == possibleInTargetRow);
}

// Zwraca 1 (true), jeśli w danym pudełku kandydat MOŻE wystąpić TYLKO w kolumnie targetCol
int isLockedToCol(char* sudoku, int boxId, int targetCol, char candidate) {
  char* box;
  getBox(sudoku, boxId, &box);
  if (contains(box, W, candidate)) {
    free(box);
    return false;
  }

  int possibleCount = 0;
  int possibleInTargetCol = 0;

  for (int i = 0; i < W; i++) {
    if (box[i] != '0') continue;
    int r = (boxId / 3) * 3 + i / 3;
    int c = (boxId % 3) * 3 + i % 3;

    char* rowStr; getRow(sudoku, r, &rowStr);
    char* colStr; getCol(sudoku, c, &colStr);

    if (!contains(rowStr, W, candidate) && !contains(colStr, W, candidate)) {
      possibleCount++;
      if (c == targetCol) {
        possibleInTargetCol++;
      }
    }
    free(rowStr); free(colStr);
  }
  free(box);

  return (possibleCount > 0 && possibleCount == possibleInTargetCol);
}


int scanningBoxes(char* sudoku, int usePointing) {
  for (int boxId = 0; boxId < BXS; boxId++) {
    if (TEST) printf("\nScanning box %d\n", boxId);
    char* box;
    getBox(sudoku, boxId, &box);

    test(box, W);

    for (int candidateIdx = 0; candidateIdx < W; candidateIdx++) {
      char candidate = candidates[candidateIdx];

      if (contains(box, W, candidate)) continue;

      if (TEST) printf("Candidate %c is not in the box\n", candidate);

      int canPlace[W];

      for (int i = 0; i < BXS; i++) {
        if (box[i] != '0') canPlace[i] = 0;
        else canPlace[i] = 1;
      }

      for (int i = 0; i < SW; i++) {
        int row = (boxId / 3) * 3 + i;
        int blockThisRow = 0; // Flaga określająca czy wiersz odpada

        // 1. Zwykłe sprawdzenie rzędu
        for (int j = 0; j < W; j++) {
          if (sudoku[row * W + j] == candidate) {
            blockThisRow = 1;
            break;
          }
        }

        // 2. Opcjonalne sprawdzenie Pointing Pairs
        if (usePointing && !blockThisRow) {
          int startBoxRow = (boxId / 3) * 3;
          for (int b = startBoxRow; b < startBoxRow + 3; b++) {
            if (b != boxId && isLockedToRow(sudoku, b, row, candidate)) {
              blockThisRow = 1; // Inne pudełko zablokowało ten rząd!
              break;
            }
          }
        }

        // Jeśli zablokowane (zwykle lub przez wskazanie), wyzeruj możliwości w pudełku
        if (blockThisRow) {
          canPlace[i * SW] = 0;
          canPlace[i * SW + 1] = 0;
          canPlace[i * SW + 2] = 0;
        }
      }

      // checking in columns
      for (int i = 0; i < SW; i++) {
        int col = (boxId % 3) * 3 + i;
        int blockThisCol = 0; // Flaga określająca czy kolumna odpada

        // 1. Zwykłe sprawdzenie kolumny
        for (int j = 0; j < W; j++) {
          if (sudoku[j * W + col] == candidate) {
            blockThisCol = 1;
            break;
          }
        }

        // 2. Opcjonalne sprawdzenie Pointing Pairs
        if (usePointing && !blockThisCol) {
          int startBoxCol = boxId % 3;
          for (int b = startBoxCol; b <= startBoxCol + 6; b += 3) {
            if (b != boxId && isLockedToCol(sudoku, b, col, candidate)) {
              blockThisCol = 1; // Inne pudełko zablokowało tę kolumnę!
              break;
            }
          }
        }

        // Jeśli zablokowane, wyzeruj możliwości w pudełku
        if (blockThisCol) {
          canPlace[i] = 0;
          canPlace[i + 3] = 0;
          canPlace[i + 6] = 0;
        }
      }

      // Zliczanie wolnych miejsc i wstawianie
      int idx = 0;
      int spacesCount = W;
      for (int i = 0; i < W; i++) {
        if (!canPlace[i]) {
          spacesCount--;
        }
        else {
          idx = i;
        }
      }

      if (spacesCount == 1) {
        int global_idx = ((boxId / 3) * 3 + (idx / 3)) * W + ((boxId % 3) * 3 + (idx % 3));
        if (TEST) printf(">>> ZNALAZLEM! Wstaw %c do pudelka %d na pozycje %d idx: %d (Pointing: %d)\n", candidate, boxId, idx, global_idx, usePointing);
        sudoku[global_idx] = candidate;
        free(box);
        return 1;
      }
    }
    free(box);
  }
  return 0;
}

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

int testbox(int method(char* sudoku)) {
  // scanning boxes test
  char sudoku[] = "042005006197000040560400109801300260900071450003256000005032700004590600000760080";
  char solution[] = "342915876197683542568427139851349267926871453473256918685132794734598621219764385";
  int try = 1;
  while (try == 1) {
    try = method(sudoku);
  }

  if (strcmp(sudoku, solution) == 0)
    return 1;
  return 0;
}

int main() {
  // 042005006
  // 197000040
  // 560400109

  // 801300260
  // 900071450
  // 003256000  

  // 005032700
  // 004590600
  // 000760080
// 
  // char* sudoku = "042005006197000040560400109801300260900071450003256000005032700004590600000760080";
  char sudoku[] = "042005006197000040560400109801300260900071450003256000005032700004590600000760080";
  int res = singleCandidate(sudokus[1]);

  char* s = sudokus[1];
  while (singleCandidate(s) || scanningBoxes(s, false) || missingInRowCol(s) || scanningBoxes(s, true))
  {
    printf("mam\n");
  }

  for (int i = 0; i < 81; i++)
  {
    printf("%c", sudokus[1][i]);
  }
  int stop = 0;
  scanf("%d", &stop);
  printf("\n");
  if (testbox(singleCandidate)) {
    printf("Test passed!\n");
  }
  else {
    printf("Test failed!\n");
  }
  // if (testbox(scanningBoxes)) {
  //   printf("Test passed!\n");
  // }
  // else {
  //   printf("Test failed!\n");
  // }
}