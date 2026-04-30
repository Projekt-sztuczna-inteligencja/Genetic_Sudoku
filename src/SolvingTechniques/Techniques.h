#ifndef SUDOKU_SOLVER_H
#define SUDOKU_SOLVER_H

#include "Utils.h"

// link do podstawowych technik: https://www.conceptispuzzles.com/index.aspx?uri=puzzle/sudoku/techniques

// Definicja typu, żeby łatwiej było robić tablice metod w main.c
typedef int (*SudokuMethod)(char* sudoku, unsigned short* masks);

typedef struct {
  char* name;
  SudokuMethod method;
  float weight;
} SudokuMethodRecord;

extern SudokuMethodRecord methodRecords[];
extern int numMethods;
// Banalne techniki 
int findHiddenSinglesInBoxes(char* sudoku, unsigned short* masks);
int findHiddenSinglesInRowsAndCols(char* sudoku, unsigned short* masks);

// Proste techniki
int findNakedSingles(char* sudoku, unsigned short* masks);
int applyPointingPairs(char* sudoku, unsigned short* masks);
int applyClaiming(char* sudoku, unsigned short* masks);

// Zaawansowane techniki 
int applyNakedPairs(char* sudoku, unsigned short* masks);
int applyHiddenPairs(char* sudoku, unsigned short* masks);
int applyUniqueRectangles(char* sudoku, unsigned short* masks);
int applyXWing(char* sudoku, unsigned short* masks);
int applyNakedTriples(char* sudoku, unsigned short* masks);
int applySwordfish(char* sudoku, unsigned short* masks);

// Eksperckie techniki
int applyHiddenTriples(char* sudoku, unsigned short* masks);
int applyXYWing(char* sudoku, unsigned short* masks);
int applyXYZWing(char* sudoku, unsigned short* masks);
int applyNakedQuads(char* sudoku, unsigned short* masks);
int applyHiddenQuads(char* sudoku, unsigned short* masks);
int applyJellyfish(char* sudoku, unsigned short* masks);
int applyWWing(char* sudoku, unsigned short* masks);


#endif // SUDOKU_SOLVER_H