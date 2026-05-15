#ifndef SUDOKU_UTILS_H
#define SUDOKU_UTILS_H

#define W 9
#define H 9
#define TEST 0 // 1 - test, 0 - production

// Deklaracje funkcji pomocniczych
int hasSingleCandidate(unsigned short mask);
int getCandidateValue(unsigned short mask);
int isCandidatePossible(unsigned short mask, int val);
int countCandidates(unsigned short mask);
int validateSudoku(char* sudoku);

void createMask(char* sudoku, unsigned short* masks);
void updateMasksAfterPlacement(unsigned short* masks, int idx, int val);
int processNakedSubset(char* sudoku, unsigned short* masks, int* indices, int n);
int processHiddenSubset(char* sudoku, unsigned short* masks, int* indices, int n);
int processHiddenQuad(char* sudoku, unsigned short* masks, int* indices);
int cellsSeeEachOther(int idx1, int idx2);


#endif // SUDOKU_UTILS_H