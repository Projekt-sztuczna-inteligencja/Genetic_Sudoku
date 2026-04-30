#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include "src/SolvingTechniques/Utils.h"
#include "src/SolvingTechniques/EasyTechniques.h"

#define true 1
#define false 0

void testAllMethods(SudokuMethod methods[], char* methodNames[], int numMethods);
void testbox(int method(char* sudoku, unsigned short* masks), char* name);

char sudokus[2][82] = {
  "042005006197000040560400109801300260900071450003256000005032700004590600000760080",
  "000102000060000070008000900400000003050007000200080001009000805070000060000304000" };
char solutions[][82] = {
    "342915876197683542568427139851349267926871453473256918685132794734598621219764385",
    "345162789261984375978357942487591623659273418123846597895417236734629581612538974" };

/*
gcc SolvingTechniques.c src/SolvingTechniques/Utils.c src/SolvingTechniques/EasyTechniques.c
*/
int main() {
  printf("\n");
  void* methods[] = { findNakedSingles, findHiddenSinglesInBoxes, findHiddenSinglesInRowsAndCols,applyPointingPairs };
  char* methodNames[] = { "Naked Single", "Hidden Single Box", "Hidden Single Row/Col","Pointing Pairs" };

  for (int i = 0; i < sizeof(methods) / sizeof(methods[0]); i++) {
    testbox(methods[i], methodNames[i]);
  }

  testAllMethods((SudokuMethod*)methods, methodNames, 4);

  return 0;
}

// ################################ Funkjce testowe ##############################

void testbox(int method(char* sudoku, unsigned short* masks), char* name) {
  // scanning boxes test
  unsigned short mask[81];
  for (int s = 0; s < 2; s++) {
    char* sudoku = strcpy((char*)malloc(82), sudokus[s]);
    char* solution = solutions[s];
    createMask(sudoku, mask);
    int try = 1;
    while (try == 1) {
      try = method(sudoku, mask);
      // printf("After method call:\n");
      // for (int i = 0; i < 9; i++) {
      //   for (int j = 0; j < 9; j++) {
      //     printf("%c ", sudoku[i * 9 + j]);
      //   }
      //   printf("\n");
      // }
    }
    if (strcmp(sudoku, solution) == 0)
      printf("Test passed for method %s on sudoku %d!\n", name, s);
    else
      printf("Test failed for method %s on sudoku %d!\n", name, s);
    free(sudoku);
  }
}
void testAllMethods(SudokuMethod methods[], char* methodNames[], int numMethods) {
  unsigned short mask[81];
  for (int s = 0; s < 2; s++) {
    printf("\n=== Rozwiazywanie Sudoku nr %d ===\n", s + 1);
    createMask(sudokus[s], mask);
    int puzzleSolved = 0;
    while (1) {
      int progressMadeThisRound = 0;
      for (int i = 0; i < numMethods - 1; i++) {
        if (methods[i](sudokus[s], mask)) {
          progressMadeThisRound = 1;
          printf("Applied method: %s\n", methodNames[i]);
          break;
        }
      }
      if (!progressMadeThisRound && numMethods > 0) {
        int lastIdx = numMethods - 1;
        if (methods[lastIdx](sudokus[s], mask)) {
          progressMadeThisRound = 1;
          printf("Applied method: %s (Metoda Redukcyjna / Ostatnia Deska Ratunku)\n", methodNames[lastIdx]);
        }
      }
      if (!progressMadeThisRound) {
        break;
      }
    }

    if (strcmp(sudokus[s], solutions[s]) == 0) {
      printf("Sukces! Sudoku %d zostalo poprawnie rozwiazane.\n", s + 1);
    }
    else {
      printf("Porazka. Solver utknal i nie potrafi rozwiazac Sudoku %d w pelni.\n", s + 1);
    }
  }
}