#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "src/Rating/SolvingTechniques/Utils.h"
#include "src/Rating/SolvingTechniques/Techniques.h"
#include "Sudokus/SudokuReader.h"
#include "src/Rating/Rater.h"

#define true 1
#define false 0

void testAllMethods(SudokuMethodRecord methodRecords[], int numMethods, SudokuRecord sudokus[], int numSudokus);
void testbox(SudokuMethodRecord methodRecord, SudokuRecord sudokus[], int numSudokus);
int validateSudoku(char* sudoku);
void rateSudokus(SudokuRecord sudokus[], int numSudokus, SudokuMethodRecord methodRecords[], int numMethods);
void runGraderDiagnostics(SudokuRecord records[], int numPuzzles, const char* datasetName, SudokuMethodRecord methodRecords[], int numMethods);


/*
*/


int main() {

  //zmierz czas 
  clock_t start = clock();
  printf("=================================================\n");
  printf("        SUDOKU SE GRADER - DIAGNOSTICS          \n");
  printf("=================================================\n");

  const char* filenames[] = {
      "Sudokus/easy.txt",
      "Sudokus/medium.txt",
      "Sudokus/hard.txt"
  };
  int numFiles = sizeof(filenames) / sizeof(filenames[0]);

  for (int i = 0; i < numFiles; i++) {
    SudokuRecord* puzzles = NULL;
    int count = readSudokusFromFile(filenames[i], &puzzles);

    if (count > 0) {
      runGraderDiagnostics(puzzles, count, filenames[i], methodRecords, numMethods);
      free(puzzles);
    }
    else {
      printf("\n[ERROR] Could not load or empty file: %s\n", filenames[i]);
    }
  }
  printf("\ntime taken: %.2f seconds\n", (double)(clock() - start) / CLOCKS_PER_SEC);
  printf("\nAll datasets processed.\n");



  return 0;
}

// ################################ Funkjce testowe ################################

void runGraderDiagnostics(SudokuRecord records[], int numPuzzles, const char* datasetName, SudokuMethodRecord methodRecords[], int numMethods) {
  printf("\n[ANALYZING] %s\n", datasetName);
  printf("-------------------------------------------------\n");

  int solvedCount = 0;
  int perfectMatches = 0;
  float totalError = 0.0f;
  float maxError = 0.0f;

  for (int i = 0; i < numPuzzles; i++) {
    // Wywołanie gradingu na digits z rekordu
    float calculatedRating = getSERating(records[i].digits, methodRecords, numMethods);

    if (calculatedRating < 0) continue; // Pomiń, jeśli nie rozwiązano

    float givenRating = atof(records[i].rating);
    float currentError = fabsf(calculatedRating - givenRating);

    solvedCount++;
    totalError += currentError;

    if (currentError < 0.01f) perfectMatches++;
    if (currentError > maxError) maxError = currentError;

    // Raportowanie dużych rozbieżności (np. błąd > 0.5)
    if (currentError > 0.5f) {
      //printf("  Mismatch [%s]: Calc: %.1f | Given: %.1f\n",
        //records[i].hash, calculatedRating, givenRating);
    }
  }

  // Obliczenia statystyczne
  if (solvedCount > 0) {
    float avgError = totalError / solvedCount;
    float successRate = ((float)solvedCount / numPuzzles) * 100.0f;
    float matchRate = ((float)perfectMatches / solvedCount) * 100.0f;

    printf("\n  Summary for %s:\n", datasetName);
    printf("  > Puzzles processed:  %d\n", numPuzzles);
    printf("  > Solved by engine:   %d (%.1f%%)\n", solvedCount, successRate);
    printf("  > Perfect SE Match:   %d (%.1f%%)\n", perfectMatches, matchRate);
    printf("  > Avg SE Deviation:   %.3f\n", avgError);
    printf("  > Max SE Deviation:   %.1f\n", maxError);
  }
  else {
    printf("  > No puzzles could be solved in this dataset.\n");
  }
  printf("-------------------------------------------------\n");
}

void testbox(SudokuMethodRecord methodRecord, SudokuRecord sudokus[], int numSudokus) {
  unsigned short mask[81];
  for (int s = 0; s < numSudokus; s++) {
    char* sudoku = strcpy((char*)malloc(82), sudokus[s].digits);
    createMask(sudoku, mask);
    int try = 1;
    while (try == 1) {
      try = methodRecord.method(sudoku, mask);
    }
    if (validateSudoku(sudoku))
      printf("Test passed for method %s on sudoku %d!\n", methodRecord.name, s);
    else
      printf("Test failed for method %s on sudoku %d!\n", methodRecord.name, s);
    free(sudoku);
  }
}

void testAllMethods(SudokuMethodRecord methodRecords[], int numMethods, SudokuRecord sudokus[], int numSudokus) {
  unsigned short mask[81];
  int solvedCount = 0;
  int hardestSudoku = 0;
  for (int s = 0; s < numSudokus; s++) {
    char currentSudoku[82];
    strcpy(currentSudoku, sudokus[s].digits);

    // printf("\n=== Rozwiazywanie Sudoku nr %d ===\n", s + 1);
    createMask(currentSudoku, mask);
    int puzzleSolved = 0;
    while (1) {
      int progressMadeThisRound = 0;
      for (int i = 0; i < numMethods - 1; i++) {
        if (methodRecords[i].method(currentSudoku, mask)) {
          progressMadeThisRound = 1;
          //printf("Applied method: %s\n", methodRecords[i].name);
          break;
        }
      }
      if (!progressMadeThisRound && numMethods > 0) {
        int lastIdx = numMethods - 1;
        if (methodRecords[lastIdx].method(currentSudoku, mask)) {
          progressMadeThisRound = 1;
          //printf("Applied method: %s\n", methodRecords[lastIdx].name);
        }
      }
      if (!progressMadeThisRound) {
        break;
      }
    }

    if (validateSudoku(currentSudoku)) {
      // printf("Sukces! Sudoku %d zostalo poprawnie rozwiazane.\n", s + 1);
      solvedCount++;
      if (hardestSudoku < (int)sudokus[s].rating) hardestSudoku = s;
    }
    else {
      // printf("Porazka. Solver utknal i nie potrafi rozwiazac Sudoku %d w pelni.\n", s + 1);
    }
  }
  printf("Liczba rozwiazanych Sudoku: %f%%\n", (float)solvedCount / numSudokus * 100);
  printf("Najtrudniejsze rozwiazane Sudoku: %d (ocena: %s)\n", hardestSudoku + 1, sudokus[hardestSudoku].rating);
}

