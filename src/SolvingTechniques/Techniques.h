#ifndef SUDOKU_SOLVER_H
#define SUDOKU_SOLVER_H

#include "Utils.h"

// Definicja typu, żeby łatwiej było robić tablice metod w main.c
typedef int (*SudokuMethod)(char* sudoku, unsigned short* masks);

typedef struct {
  char* name;
  SudokuMethod method;
  float weight;
} SudokuMethodRecord;

SudokuMethodRecord methodRecords[];
int numMethods;
// Proste techniki 
int findNakedSingles(char* sudoku, unsigned short* masks);
int findHiddenSinglesInBoxes(char* sudoku, unsigned short* masks);
int findHiddenSinglesInRowsAndCols(char* sudoku, unsigned short* masks);
int applyPointingPairs(char* sudoku, unsigned short* masks);

// Zaawansowane techniki 
int applyNakedPairs(char* sudoku, unsigned short* masks);
int applyHiddenPairs(char* sudoku, unsigned short* masks);
int applyXWing(char* sudoku, unsigned short* masks);

#endif // SUDOKU_SOLVER_H