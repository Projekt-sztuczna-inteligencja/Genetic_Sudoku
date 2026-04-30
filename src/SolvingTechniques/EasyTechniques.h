#ifndef SUDOKU_SOLVER_H
#define SUDOKU_SOLVER_H

#include "Utils.h"

// Definicja typu, żeby łatwiej było robić tablice metod w main.c
typedef int (*SudokuMethod)(char* sudoku, unsigned short* masks);

// Deklaracje naszych 4 głównych technik
int findNakedSingles(char* sudoku, unsigned short* masks);
int findHiddenSinglesInBoxes(char* sudoku, unsigned short* masks);
int findHiddenSinglesInRowsAndCols(char* sudoku, unsigned short* masks);
int applyPointingPairs(char* sudoku, unsigned short* masks);

#endif // SUDOKU_SOLVER_H