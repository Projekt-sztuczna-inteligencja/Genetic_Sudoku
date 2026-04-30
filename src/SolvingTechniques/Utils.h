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

void createMask(char* sudoku, unsigned short* masks);
void updateMasksAfterPlacement(unsigned short* masks, int idx, int val);

#endif // SUDOKU_UTILS_H