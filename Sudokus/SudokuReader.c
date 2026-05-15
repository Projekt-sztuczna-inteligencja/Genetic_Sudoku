#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SudokuReader.h"

int readSudokusFromFile(const char* filename, SudokuRecord** records) {
  FILE* file = fopen(filename, "r");
  if (!file) {
    printf("Błąd: Nie można otworzyć pliku %s\n", filename);
    return -1;
  }

  int capacity = 1000; // Początkowy rozmiar tablicy
  int count = 0;
  *records = (SudokuRecord*)malloc(capacity * sizeof(SudokuRecord));

  char line[128]; // Bufor z zapasem (linia ma 100 bajtów)

  while (fgets(line, sizeof(line), file)) {
    // Jeśli linia jest pusta lub za krótka, pomijamy ją
    if (strlen(line) < 90) continue;

    // Jeśli brakuje miejsca w tablicy, podwajamy jej rozmiar
    if (count >= capacity) {
      capacity *= 2;
      *records = (SudokuRecord*)realloc(*records, capacity * sizeof(SudokuRecord));
    }

    // Pobieramy 3 wartości rozdzielone spacją. 
    // Zabezpieczamy maksymalną długość stringów by nie przepełnić buforów w structcie
    if (sscanf(line, "%12s %81s %4s",
      (*records)[count].hash,
      (*records)[count].digits,
      (*records)[count].rating) == 3) {

      count++; // Pomyślnie wczytano rekord
    }
  }

  fclose(file);
  return count;
}