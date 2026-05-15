#ifndef SUDOKU_READER_H
#define SUDOKU_READER_H

typedef struct {
  char hash[13];     // 12 znaków + null terminator (\0)
  char digits[82];   // 81 znaków planszy + \0
  char rating[5];    // 4 znaki oceny (np. " 2.5") + \0
} SudokuRecord;

// Funkcja wczytująca plik. Zwraca liczbę wczytanych plansz (lub -1 w razie błędu).
// Pamięć dla tablicy 'records' jest alokowana dynamicznie wewnątrz funkcji.
int readSudokusFromFile(const char* filename, SudokuRecord** records);

#endif // SUDOKU_READER_H