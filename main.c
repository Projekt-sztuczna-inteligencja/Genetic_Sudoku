#include <stdio.h>
#include <stdlib.h>

#define W 9
#define H 9
#define SW 3
#define BXS 9

#define TEST 0 // 1 - test, 0 - production

char candidates[] = {
  '1', '2', '3', '4', '5', '6', '7', '8', '9'
};

float dificulty(char* sudoku) {

}

int conatains(char* tab, int size, char value) {
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

int scanningBoxes(char* sudoku) {
  for (int boxId = 0; boxId < BXS; boxId++) {
    if (TEST) printf("\nScanning box %d\n", boxId);
    char* box = (char*)malloc(sizeof(char) * W);

    for (int i = 0; i < W; i++) {
      int row = (boxId / 3) * 3 + i / 3;
      int col = (boxId % 3) * 3 + i % 3;
      box[i] = sudoku[row * W + col];
    }
    test(box, W);

    for (int candidateIdx = 0; candidateIdx < W; candidateIdx++) {
      char candidate = candidates[candidateIdx];

      if (conatains(box, W, candidate)) continue;

      if (TEST) printf("Candidate %c is not in the box\n", candidate);

      int* canPlace = (int*)malloc(sizeof(int) * W);

      // checking if place is already taken
      for (int i = 0; i < BXS; i++) {
        if (box[i] != '0') canPlace[i] = 0;
        else canPlace[i] = 1;
      }

      // checking in rows
      for (int i = 0; i < SW; i++) {
        int row = (boxId / 3) * 3 + i;
        for (int j = 0; j < W; j++) {
          if (sudoku[row * W + j] == candidate) {
            canPlace[i * SW] = 0;
            canPlace[i * SW + 1] = 0;
            canPlace[i * SW + 2] = 0;
            break;
          }
        }
      }

      // checking in columns
      for (int i = 0; i < SW; i++) {
        int col = (boxId % 3) * 3 + i;
        for (int j = 0; j < W; j++) {
          if (sudoku[j * W + col] == candidate) {
            canPlace[i] = 0;
            canPlace[i + 3] = 0;
            canPlace[i + 6] = 0;
            break;
          }
        }
      }

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
        printf(">>> ZNALAZLEM! Wstaw %c do pudelka %d na pozycje %d idx: %d\n", candidate, boxId, idx, global_idx);
        sudoku[global_idx] = candidate;
        free(canPlace);
        free(box);
        return 1;
      }
      free(canPlace);
    }
    free(box);
  }
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

  // char* sudoku = "042005006197000040560400109801300260900071450003256000005032700004590600000760080";
  char sudoku[] = "342005006197603040568407109801300260900071450003256000005032700004590600000760080";
  int res = 1;
  while (res == 1)
  {
    res = scanningBoxes(sudoku);
  }
  for (int i = 0; i < 81; i++)
  {
    printf("%c ", sudoku[i]);
  }
  printf("\n");
}