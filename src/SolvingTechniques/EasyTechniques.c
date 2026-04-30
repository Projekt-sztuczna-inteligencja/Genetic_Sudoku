#include "EasyTechniques.h"
#include <stdio.h>
// ############################## Metody rozwiązywania ##############################

/* =====================================================================
 * Technika 1 (Scanning in one direction) &
 * Technika 2 (Scanning in two directions)
 *
 * Nazwa w kodzie: findHiddenSinglesInBoxes (Ukryty Pojedynczy w pudełku)
 * Jak to działa:
 * Funkcja skanuje każde z 9 pudełek. Dla każdej cyfry (od 1 do 9) liczy,
 * w ilu wolnych komórkach wewnątrz danego pudełka można ją wpisać
 * (sprawdzając maski bitowe). Jeśli licznik wskaże, że jest tylko JEDNO
 * takie miejsce w całym pudełku, funkcja od razu wstawia tam tę cyfrę.
 * ===================================================================== */

int findHiddenSinglesInBoxes(char* sudoku, unsigned short* masks) {
  int changed = 0;
  for (int boxId = 0; boxId < 9; boxId++) {
    int boxStartRow = (boxId / 3) * 3;
    int boxStartCol = (boxId % 3) * 3;

    // Sprawdzamy każdą cyfrę od 1 do 9
    for (int val = 1; val <= 9; val++) {
      int possibleCount = 0;
      int lastPossibleIdx = -1;

      // Skanujemy 9 komórek w pudełku
      for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 3; c++) {
          int idx = (boxStartRow + r) * 9 + (boxStartCol + c);

          if (sudoku[idx] == '0' && isCandidatePossible(masks[idx], val)) {
            possibleCount++;
            lastPossibleIdx = idx;
          }
        }
      }
      if (possibleCount == 1) {
        sudoku[lastPossibleIdx] = val + '0';
        if (TEST) printf(">>> ZNALAZLEM (Hidden Single Box)! Wstaw %d na pozycje %d\n", val, lastPossibleIdx);

        updateMasksAfterPlacement(masks, lastPossibleIdx, val);
        changed = 1;
      }
    }
  }
  return changed;
}

/* =====================================================================
 * Technika 3 (Searching for Single Candidates)
 *
 * Nazwa w kodzie: findNakedSingles (Nagi Pojedynczy)
 * Jak to działa:
 * Funkcja analizuje każdą pustą komórkę na planszy z osobna. Odczytuje jej
 * maskę bitową, która zawiera wynik wszystkich dotychczasowych eliminacji
 * (z wiersza, kolumny i pudełka). Jeśli w masce został zapalony tylko
 * jeden bit (pozostałych 8 kandydatów zostało wykreślonych), funkcja
 * odczytuje tę jedyną możliwą cyfrę i wstawia ją na planszę.
 * ===================================================================== */

int findNakedSingles(char* sudoku, unsigned short* masks) { // metoda Naked Single
  int changed = 0;
  for (int i = 0; i < 81; i++) {
    if (sudoku[i] == '0' && hasSingleCandidate(masks[i])) {
      int val = getCandidateValue(masks[i]);
      sudoku[i] = val + '0'; // Wstawienie znaku do planszy
      if (TEST) printf(">>> ZNALAZLEM (Naked Single)! Wstaw %d na pozycje %d\n", val, i);

      // TU BARDZO WAŻNE: Musisz zaktualizować (wyczyścić) maski w rzędzie, kolumnie i pudełku!
      updateMasksAfterPlacement(masks, i, val);

      changed = 1;
    }
  }
  return changed;
}


/* =====================================================================
 * Technika 4 (Eliminating numbers from rows, columns and boxes)
 *
 * Nazwa w kodzie: applyPointingPairs (Wskazujące Pary / Redukcja Liniowa)
 * Jak to działa:
 * Jest to technika filtrująca (nigdy nie wstawia cyfr na planszę).
 * Skanuje pudełka i szuka sytuacji, gdzie wszystkie możliwe miejsca dla
 * danego kandydata leżą w jednej linii (w tym samym wierszu lub kolumnie).
 * Oznacza to, że pudełko "rezerwuje" ten fragment linii dla tej cyfry.
 * Funkcja "wymazuje" (zeruje bit) tego kandydata z reszty danego wiersza
 * lub kolumny w pozostałych pudełkach, odblokowując łatwiejsze metody.
 * ===================================================================== */

int applyPointingPairs(char* sudoku, unsigned short* masks) {
  int masksChanged = 0;

  for (int boxId = 0; boxId < 9; boxId++) {
    int boxStartRow = (boxId / 3) * 3;
    int boxStartCol = (boxId % 3) * 3;

    // Sprawdzamy każdego kandydata od 1 do 9
    for (int val = 1; val <= 9; val++) {
      int possibleInRow[3] = { 0, 0, 0 }; // Zlicza kandydatów w 3 wierszach pudełka
      int possibleInCol[3] = { 0, 0, 0 }; // Zlicza kandydatów w 3 kolumnach pudełka
      int totalPossible = 0;

      // KROK 1: Skanowanie pudełka
      for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 3; c++) {
          int idx = (boxStartRow + r) * 9 + (boxStartCol + c);
          // isCandidatePossible to funkcja pomocnicza z poprzedniej wiadomości
          if (sudoku[idx] == '0' && isCandidatePossible(masks[idx], val)) {
            possibleInRow[r]++;
            possibleInCol[c]++;
            totalPossible++;
          }
        }
      }

      // Jeśli jest tylko 1 lub wcale, to nie jest to "Para Wskazująca" (albo pole odpada)
      if (totalPossible <= 1) continue;

      unsigned short bitToClear = ~(1 << (val - 1));

      // KROK 2A: Sprawdzamy czy kandydat jest zablokowany w jednym WIERSZU
      for (int r = 0; r < 3; r++) {
        // Jeśli wszystkie opcje dla tego kandydata leżą w tym jednym rzędzie:
        if (possibleInRow[r] == totalPossible) {
          int globalRow = boxStartRow + r;

          // Skanujemy CAŁY globalny wiersz (od col 0 do 8)
          for (int c = 0; c < 9; c++) {
            int cBox = (globalRow / 3) * 3 + (c / 3);
            int idx = globalRow * 9 + c;

            // Czyścimy maski, ale POMIJAMY nasze startowe pudełko!
            if (cBox != boxId && sudoku[idx] == '0' && isCandidatePossible(masks[idx], val)) {
              masks[idx] &= bitToClear; // Wyzerowanie bitu
              masksChanged = 1;
              if (TEST) printf(">>> Pointing Pairs: Usunieto %d z row %d, col %d (zablokowane w pudelku %d)\n", val, globalRow, c, boxId);
            }
          }
        }
      }

      // KROK 2B: Sprawdzamy czy kandydat jest zablokowany w jednej KOLUMNIE
      for (int c = 0; c < 3; c++) {
        // Jeśli wszystkie opcje dla tego kandydata leżą w tej jednej kolumnie:
        if (possibleInCol[c] == totalPossible) {
          int globalCol = boxStartCol + c;

          // Skanujemy CAŁĄ globalną kolumnę (od row 0 do 8)
          for (int r = 0; r < 9; r++) {
            int rBox = (r / 3) * 3 + (globalCol / 3);
            int idx = r * 9 + globalCol;

            // Czyścimy maski, ale POMIJAMY nasze startowe pudełko!
            if (rBox != boxId && sudoku[idx] == '0' && isCandidatePossible(masks[idx], val)) {
              masks[idx] &= bitToClear; // Wyzerowanie bitu
              masksChanged = 1;
              if (TEST) printf(">>> Pointing Pairs: Usunieto %d z row %d, col %d (zablokowane w pudelku %d)\n", val, r, globalCol, boxId);
            }
          }
        }
      }
    }
  }
  return masksChanged;
}

/* =====================================================================
 * Technika 5 (Searching for missing numbers in rows and columns)
 *
 * Nazwa w kodzie: findHiddenSinglesInRowsAndCols (Ukryty Pojed w Liniach)
 * Jak to działa:
 * Funkcja skanuje niezależnie każdy wiersz oraz każdą kolumnę.
 * Dla każdej cyfry (od 1 do 9) sprawdza, w ilu wolnych miejscach w danej
 * linii może ona zostać wpisana (na podstawie masek bitowych).
 * Jeśli w całym wierszu (lub kolumnie) jest tylko JEDNO miejsce
 * pozwalające na wpisanie danej cyfry, to musi ona tam trafić.
 * Funkcja wstawia cyfrę i od razu aktualizuje maski sąsiadów.
 * ===================================================================== */
int findHiddenSinglesInRowsAndCols(char* sudoku, unsigned short* masks) {
  int changed = 0;

  // 1. SKANOWANIE WIERSZY
  for (int row = 0; row < 9; row++) {
    for (int val = 1; val <= 9; val++) {
      int possibleCount = 0;
      int lastPossibleIdx = -1;

      for (int col = 0; col < 9; col++) {
        int idx = row * 9 + col;

        // Jeśli pole puste i maska pozwala na tę cyfrę
        if (sudoku[idx] == '0' && isCandidatePossible(masks[idx], val)) {
          possibleCount++;
          lastPossibleIdx = idx;
        }
      }

      // Znaleźliśmy dokładnie jedno miejsce w całym wierszu!
      if (possibleCount == 1) {
        sudoku[lastPossibleIdx] = val + '0';
        if (TEST) printf(">>> ZNALAZLEM (Hidden Single Row)! Wstaw %d na pozycje %d (wiersz %d)\n", val, lastPossibleIdx, row);

        // Bardzo ważne: od razu aktualizujemy maski!
        updateMasksAfterPlacement(masks, lastPossibleIdx, val);
        changed = 1;
      }
    }
  }

  // 2. SKANOWANIE KOLUMN
  for (int col = 0; col < 9; col++) {
    for (int val = 1; val <= 9; val++) {
      int possibleCount = 0;
      int lastPossibleIdx = -1;

      for (int row = 0; row < 9; row++) {
        int idx = row * 9 + col;

        if (sudoku[idx] == '0' && isCandidatePossible(masks[idx], val)) {
          possibleCount++;
          lastPossibleIdx = idx;
        }
      }

      // Znaleźliśmy dokładnie jedno miejsce w całej kolumnie!
      if (possibleCount == 1) {
        // Zabezpieczenie: sprawdzamy, czy wiersze u góry nie zdążyły już tego wypełnić
        if (sudoku[lastPossibleIdx] == '0') {
          sudoku[lastPossibleIdx] = val + '0';
          if (TEST) printf(">>> ZNALAZLEM (Hidden Single Col)! Wstaw %d na pozycje %d (kolumna %d)\n", val, lastPossibleIdx, col);

          updateMasksAfterPlacement(masks, lastPossibleIdx, val);
          changed = 1;
        }
      }
    }
  }

  return changed;
}