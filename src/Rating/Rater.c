#include "Rater.h"
#include <string.h>


// Przypisane oficjalne wagi SE Rating


// Zwraca SE Rating jako float (np. 3.2). 
// Jeśli Sudoku jest za trudne dla naszych metod, zwraca -1.0
// Funkcja wyliczająca trudność Sudoku na podstawie systemu SE Rating
float getSERating(char* originalSudoku, SudokuMethodRecord records[], int methodsCount) {
  char sudoku[82];
  strcpy(sudoku, originalSudoku); // Pracujemy na kopii

  unsigned short mask[81];
  createMask(sudoku, mask);

  float maxSE = 1.0f; // Bazowy poziom trudności

  while (1) {
    int progress = 0;

    // Iterujemy przez tablicę struktur
    for (int i = 0; i < methodsCount; i++) {
      // Wywołujemy funkcję ukrytą w strukturze
      if (records[i].method(sudoku, mask)) {

        // Jeśli użyliśmy metody, sprawdzamy jej wagę SE
        if (records[i].weight > maxSE) {
          maxSE = records[i].weight;
        }

        progress = 1;

        // Przerywamy, by zacząć od najłatwiejszych metod
        break;
      }
    }

    // Zacięcie - żadna metoda nic nie zmieniła
    if (!progress) {
      break;
    }
  }

  // Walidacja
  if (validateSudoku(sudoku)) {
    return maxSE;
  }
  else {
    return -1.0f; // Utknęliśmy, potrzebne trudniejsze metody
  }
}