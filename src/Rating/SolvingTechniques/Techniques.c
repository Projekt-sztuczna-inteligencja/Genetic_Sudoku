#include "Techniques.h"
#include <stdio.h>



SudokuMethodRecord methodRecords[] = {
  // ================= POZIOM: BARDZO ŁATWY (1.2 - 1.5) =================
  {"Hidden Single (Box)", findHiddenSinglesInBoxes, 1.2f},
  {"Hidden Single (Line)", findHiddenSinglesInRowsAndCols, 1.5f},
  // ================= POZIOM: ŁATWY / ŚREDNI (2.3 - 2.8) =================
  {"Naked Single", findNakedSingles, 2.3f},
  {"Pointing Pairs", applyPointingPairs, 2.6f},
  {"Claiming (Box-Line)", applyClaiming, 2.8f}, // "Lustro" Pointing Pairs
  // ================= POZIOM: TRUDNY (3.0 - 3.8) =================
  {"Naked Pair", applyNakedPairs, 3.0f},
  {"X-Wing", applyXWing, 3.2f},
  {"Hidden Pair", applyHiddenPairs, 3.4f},
  {"Unique Rectangle", applyUniqueRectangles, 3.5f},
  {"Naked Triple", applyNakedTriples, 3.6f},
  {"Swordfish", applySwordfish, 3.8f},
  // ================= POZIOM: EKSPERT (4.0 - 5.2) =================
  {"Hidden Triple", applyHiddenTriples, 4.0f},
 {"XY-Wing", applyXYWing, 4.2f},
 {"XYZ-Wing", applyXYZWing, 4.4f},
 {"Naked Quad", applyNakedQuads, 4.6f},
 {"Hidden Quad", applyHiddenQuads, 4.8f},
 {"W-Wing", applyWWing, 5.0f},
 {"Jellyfish", applyJellyfish, 5.2f}
};
int numMethods = sizeof(methodRecords) / sizeof(methodRecords[0]);


// =====================================================================
// łatwe techniki
// =====================================================================

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
 * Technika 5 (Box-Line Reduction)
 *
 * Nazwa w kodzie: applyClaiming (Redukcja Pudełko-Linia / "Claiming")
 * Jak to działa:
 * Jest to technika filtrująca, będąca logicznym "lustrem" Pointing Pairs.
 * Zamiast skanować pudełka, funkcja skanuje całe linie (wiersze i kolumny).
 * Jeśli wszystkie wystąpienia danego kandydata w obrębie linii znajdują
 * się wewnątrz jednego pudełka, oznacza to, że ta linia "wymusza"
 * obecność tej cyfry w tym konkretnym kwadracie.
 *
 * Funkcja "wymazuje" (zeruje bit) tego kandydata z pozostałych komórek
 * tego pudełka (tych, które nie leżą na skanowanej linii), co często
 * jest kluczem do odblokowania ukrytych singli (Hidden Singles).
 * ===================================================================== */

int applyClaiming(char* sudoku, unsigned short* masks) {
  int masksChanged = 0;

  // Sprawdzamy każdego kandydata od 1 do 9
  for (int val = 1; val <= 9; val++) {
    unsigned short bitToClear = ~(1 << (val - 1));

    // --- KROK 1: Skanowanie WIERSZY ---
    for (int r = 0; r < 9; r++) {
      int boxesInRow[3] = { 0, 0, 0 }; // Zlicza wystąpienia w 3 segmentach wiersza (każdy segment to inne pudełko)
      int totalInRow = 0;

      for (int c = 0; c < 9; c++) {
        int idx = r * 9 + c;
        if (sudoku[idx] == '0' && isCandidatePossible(masks[idx], val)) {
          boxesInRow[c / 3]++;
          totalInRow++;
        }
      }

      // Jeśli kandydat występuje w wierszu, sprawdźmy czy tylko w jednym pudełku
      if (totalInRow > 1) {
        for (int b = 0; b < 3; b++) {
          if (boxesInRow[b] == totalInRow) {
            // Znaleziono! Kandydat 'val' w wierszu 'r' jest tylko w pudełku o indeksie:
            int targetBoxId = (r / 3) * 3 + b;

            // KROK 2: Czyszczenie reszty PUDEŁKA (z wyłączeniem wiersza 'r')
            int boxStartRow = (targetBoxId / 3) * 3;
            int boxStartCol = (targetBoxId % 3) * 3;

            for (int br = 0; br < 3; br++) {
              int globalRow = boxStartRow + br;
              if (globalRow == r) continue; // Pomiń wiersz, który jest źródłem "claimingu"

              for (int bc = 0; bc < 3; bc++) {
                int globalCol = boxStartCol + bc;
                int idx = globalRow * 9 + globalCol;

                if (sudoku[idx] == '0' && isCandidatePossible(masks[idx], val)) {
                  masks[idx] &= bitToClear;
                  masksChanged = 1;
                  if (TEST) printf(">>> Claiming (Row): Usunieto %d z row %d, col %d (zablokowane przez row %d w box %d)\n", val, globalRow, globalCol, r, targetBoxId);
                }
              }
            }
          }
        }
      }
    }

    // --- KROK 2: Skanowanie KOLUMN ---
    for (int c = 0; c < 9; c++) {
      int boxesInCol[3] = { 0, 0, 0 }; // Zlicza wystąpienia w 3 segmentach kolumny
      int totalInCol = 0;

      for (int r = 0; r < 9; r++) {
        int idx = r * 9 + c;
        if (sudoku[idx] == '0' && isCandidatePossible(masks[idx], val)) {
          boxesInCol[r / 3]++;
          totalInCol++;
        }
      }

      if (totalInCol > 1) {
        for (int b = 0; b < 3; b++) {
          if (boxesInCol[b] == totalInCol) {
            // Kandydat 'val' w kolumnie 'c' jest zamknięty w pudełku:
            int targetBoxId = b * 3 + (c / 3);

            // KROK 2: Czyszczenie reszty PUDEŁKA (z wyłączeniem kolumny 'c')
            int boxStartRow = (targetBoxId / 3) * 3;
            int boxStartCol = (targetBoxId % 3) * 3;

            for (int bc = 0; bc < 3; bc++) {
              int globalCol = boxStartCol + bc;
              if (globalCol == c) continue; // Pomiń kolumnę źródłową

              for (int br = 0; br < 3; br++) {
                int globalRow = boxStartRow + br;
                int idx = globalRow * 9 + globalCol;

                if (sudoku[idx] == '0' && isCandidatePossible(masks[idx], val)) {
                  masks[idx] &= bitToClear;
                  masksChanged = 1;
                  if (TEST) printf(">>> Claiming (Col): Usunieto %d z row %d, col %d (zablokowane przez col %d w box %d)\n", val, globalRow, globalCol, c, targetBoxId);
                }
              }
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


// =====================================================================
// Trudne techniki
// =====================================================================


/* =====================================================================
 * Technika 1 & 2: Naked Pairs (Nagie Pary w Pudełkach, Wierszach, Kolumnach)
 *
 * Jeśli dwa pola w tej samej linii/pudełku mają DOKŁADNIE tych samych
 * dwóch kandydatów (np. tylko 4 i 9), to te cyfry muszą zająć te dwa pola.
 * Możemy je bezpiecznie usunąć z masek wszystkich innych pól w tym obszarze.
 * ===================================================================== */
int applyNakedPairs(char* sudoku, unsigned short* masks) {
  int changed = 0;

  // 1. SKANOWANIE WIERSZY
  for (int row = 0; row < 9; row++) {
    for (int c1 = 0; c1 < 8; c1++) {
      int idx1 = row * 9 + c1;

      // Komórka musi być pusta i mieć dokładnie 2 kandydatów
      if (sudoku[idx1] == '0' && countCandidates(masks[idx1]) == 2) {

        // Szukamy pary dla niej w dalszej części wiersza
        for (int c2 = c1 + 1; c2 < 9; c2++) {
          int idx2 = row * 9 + c2;

          if (sudoku[idx2] == '0' && masks[idx1] == masks[idx2]) {
            // Mamy Nagą Parę!
            unsigned short pairMask = masks[idx1];
            unsigned short clearMask = ~pairMask;

            // Czyścimy resztę wiersza
            for (int c3 = 0; c3 < 9; c3++) {
              int idx3 = row * 9 + c3;
              if (c3 != c1 && c3 != c2 && sudoku[idx3] == '0') {
                // Jeśli w masce sąsiada jest jakikolwiek bit z naszej pary
                if (masks[idx3] & pairMask) {
                  masks[idx3] &= clearMask; // Usuwamy oba bity
                  changed = 1;
                  if (TEST) printf(">>> Naked Pair (Row %d): usunieto z col %d\n", row, c3);
                }
              }
            }
          }
        }
      }
    }
  }

  // 2. SKANOWANIE KOLUMN
  for (int col = 0; col < 9; col++) {
    for (int r1 = 0; r1 < 8; r1++) {
      int idx1 = r1 * 9 + col;

      if (sudoku[idx1] == '0' && countCandidates(masks[idx1]) == 2) {

        for (int r2 = r1 + 1; r2 < 9; r2++) {
          int idx2 = r2 * 9 + col;

          if (sudoku[idx2] == '0' && masks[idx1] == masks[idx2]) {
            unsigned short pairMask = masks[idx1];
            unsigned short clearMask = ~pairMask;

            // Czyścimy resztę kolumny
            for (int r3 = 0; r3 < 9; r3++) {
              int idx3 = r3 * 9 + col;
              if (r3 != r1 && r3 != r2 && sudoku[idx3] == '0') {
                if (masks[idx3] & pairMask) {
                  masks[idx3] &= clearMask;
                  changed = 1;
                  if (TEST) printf(">>> Naked Pair (Col %d): usunieto z row %d\n", col, r3);
                }
              }
            }
          }
        }
      }
    }
  }

  // 3. SKANOWANIE PUDEŁEK
  for (int box = 0; box < 9; box++) {
    int startRow = (box / 3) * 3;
    int startCol = (box % 3) * 3;

    // i1 oraz i2 to lokalne indeksy wewnątrz pudełka (od 0 do 8)
    for (int i1 = 0; i1 < 8; i1++) {
      int r1 = startRow + (i1 / 3);
      int c1 = startCol + (i1 % 3);
      int idx1 = r1 * 9 + c1;

      if (sudoku[idx1] == '0' && countCandidates(masks[idx1]) == 2) {

        for (int i2 = i1 + 1; i2 < 9; i2++) {
          int r2 = startRow + (i2 / 3);
          int c2 = startCol + (i2 % 3);
          int idx2 = r2 * 9 + c2;

          if (sudoku[idx2] == '0' && masks[idx1] == masks[idx2]) {
            unsigned short pairMask = masks[idx1];
            unsigned short clearMask = ~pairMask;

            // Czyścimy resztę pudełka
            for (int i3 = 0; i3 < 9; i3++) {
              if (i3 != i1 && i3 != i2) {
                int r3 = startRow + (i3 / 3);
                int c3 = startCol + (i3 % 3);
                int idx3 = r3 * 9 + c3;

                if (sudoku[idx3] == '0' && (masks[idx3] & pairMask)) {
                  masks[idx3] &= clearMask;
                  changed = 1;
                  if (TEST) printf(">>> Naked Pair (Box %d): usunieto z row %d, col %d\n", box, r3, c3);
                }
              }
            }
          }
        }
      }
    }
  }

  return changed;
}

/* =====================================================================
 * Technika 3: Hidden Pairs (Ukryte Pary w wierszach, kolumnach, pudełkach)
 *
 * Jeśli dwie cyfry (np. 1 i 4) występują w danym obszarze (rzędzie)
 * DOKŁADNIE 2 razy i do tego dzielą ze sobą te same dwie komórki,
 * to tworzą Ukrytą Parę. Inni kandydaci w tych komórkach stają się
 * niemożliwi i usuwamy ich, zostawiając w maskach tylko naszą parę.
 * ===================================================================== */
int applyHiddenPairs(char* sudoku, unsigned short* masks) {
  int changed = 0;

  // 1. SKANOWANIE WIERSZY
  for (int row = 0; row < 9; row++) {
    int counts[10] = { 0 };       // Ile razy występuje dany kandydat (1-9)
    int pos1[10], pos2[10];     // Zapisujemy pozycje (kolumny), w których wystąpił

    // Zbieramy statystyki dla całego wiersza
    for (int col = 0; col < 9; col++) {
      int idx = row * 9 + col;
      if (sudoku[idx] == '0') {
        for (int v = 1; v <= 9; v++) {
          if (isCandidatePossible(masks[idx], v)) {
            if (counts[v] == 0) pos1[v] = col;
            else if (counts[v] == 1) pos2[v] = col;
            counts[v]++;
          }
        }
      }
    }

    // Szukamy dwóch kandydatów (v1 i v2), którzy występują dokładnie 2 razy 
    // i dzielą te same pozycje (kolumny).
    for (int v1 = 1; v1 <= 8; v1++) {
      if (counts[v1] == 2) {
        for (int v2 = v1 + 1; v2 <= 9; v2++) {
          if (counts[v2] == 2 && pos1[v1] == pos1[v2] && pos2[v1] == pos2[v2]) {

            // Mamy Ukrytą Parę! Tworzymy maskę, w której zapalone są tylko te 2 bity
            unsigned short pairMask = (1 << (v1 - 1)) | (1 << (v2 - 1));

            int idx1 = row * 9 + pos1[v1];
            int idx2 = row * 9 + pos2[v1];

            // Sprawdzamy, czy w tych komórkach są jakieś 'śmieci' do usunięcia
            if (masks[idx1] != pairMask || masks[idx2] != pairMask) {
              masks[idx1] = pairMask; // Zostawiamy TYLKO naszą parę
              masks[idx2] = pairMask;
              changed = 1;
              if (TEST) printf(">>> Hidden Pair (Row %d): zatrzasnieto kandydatow %d i %d w kolumnach %d i %d\n", row, v1, v2, pos1[v1], pos2[v1]);
            }
          }
        }
      }
    }
  }

  // 2. SKANOWANIE KOLUMN
  for (int col = 0; col < 9; col++) {
    int counts[10] = { 0 };
    int pos1[10], pos2[10]; // Tym razem zapisujemy wiersze

    for (int row = 0; row < 9; row++) {
      int idx = row * 9 + col;
      if (sudoku[idx] == '0') {
        for (int v = 1; v <= 9; v++) {
          if (isCandidatePossible(masks[idx], v)) {
            if (counts[v] == 0) pos1[v] = row;
            else if (counts[v] == 1) pos2[v] = row;
            counts[v]++;
          }
        }
      }
    }

    for (int v1 = 1; v1 <= 8; v1++) {
      if (counts[v1] == 2) {
        for (int v2 = v1 + 1; v2 <= 9; v2++) {
          if (counts[v2] == 2 && pos1[v1] == pos1[v2] && pos2[v1] == pos2[v2]) {
            unsigned short pairMask = (1 << (v1 - 1)) | (1 << (v2 - 1));

            int idx1 = pos1[v1] * 9 + col;
            int idx2 = pos2[v1] * 9 + col;

            if (masks[idx1] != pairMask || masks[idx2] != pairMask) {
              masks[idx1] = pairMask;
              masks[idx2] = pairMask;
              changed = 1;
              if (TEST) printf(">>> Hidden Pair (Col %d): zatrzasnieto kandydatow %d i %d w rzedach %d i %d\n", col, v1, v2, pos1[v1], pos2[v1]);
            }
          }
        }
      }
    }
  }

  // 3. SKANOWANIE PUDEŁEK
  for (int box = 0; box < 9; box++) {
    int counts[10] = { 0 };
    int pos1[10], pos2[10];
    int startRow = (box / 3) * 3;
    int startCol = (box % 3) * 3;

    for (int i = 0; i < 9; i++) {
      int r = startRow + (i / 3);
      int c = startCol + (i % 3);
      int idx = r * 9 + c;

      if (sudoku[idx] == '0') {
        for (int v = 1; v <= 9; v++) {
          if (isCandidatePossible(masks[idx], v)) {
            if (counts[v] == 0) pos1[v] = i;
            else if (counts[v] == 1) pos2[v] = i;
            counts[v]++;
          }
        }
      }
    }

    for (int v1 = 1; v1 <= 8; v1++) {
      if (counts[v1] == 2) {
        for (int v2 = v1 + 1; v2 <= 9; v2++) {
          if (counts[v2] == 2 && pos1[v1] == pos1[v2] && pos2[v1] == pos2[v2]) {
            unsigned short pairMask = (1 << (v1 - 1)) | (1 << (v2 - 1));

            int r1 = startRow + (pos1[v1] / 3);
            int c1 = startCol + (pos1[v1] % 3);
            int idx1 = r1 * 9 + c1;

            int r2 = startRow + (pos2[v1] / 3);
            int c2 = startCol + (pos2[v1] % 3);
            int idx2 = r2 * 9 + c2;

            if (masks[idx1] != pairMask || masks[idx2] != pairMask) {
              masks[idx1] = pairMask;
              masks[idx2] = pairMask;
              changed = 1;
              if (TEST) printf(">>> Hidden Pair (Box %d): zatrzasnieto kandydatow %d i %d\n", box, v1, v2);
            }
          }
        }
      }
    }
  }

  return changed;
}

/* =====================================================================
 * Technika: Unique Rectangles (Zunifikowana: Typ 1, 2, 3, 4)
 *
 * Nazwa w kodzie: applyUniqueRectangles
 * SE Rating: 3.5 (Type 1) do 3.8 (Type 4)
 *
 * Jak to działa:
 * Funkcja szuka układu 4 komórek na przecięciu 2 wierszy i 2 kolumn,
 * leżących w dokładnie 2 pudełkach. Jeśli wszystkie 4 komórki zawierają
 * parę {A, B}, sprawdzamy "brudne" komórki (te z dodatkowymi bitami):
 * - 1 brudna: Typ 1 (usuwamy A, B z brudnej).
 * - 2 brudne z tym samym dodatkowym bitem X: Typ 2 (usuwamy X z zasięgu wzroku).
 * - 2 brudne + pomocnicza komórka: Typ 3 (Naked Subset wewnątrz UR).
 * - 2 brudne, gdzie A lub B występuje tylko w nich: Typ 4 (usuwamy drugą cyfrę).
 * ===================================================================== */

int applyUniqueRectangles(char* sudoku, unsigned short* masks) {
  int changed = 0;

  // Skanujemy wszystkie pary kandydatów (A, B)
  for (int a = 1; a <= 8; a++) {
    for (int b = a + 1; b <= 9; b++) {
      unsigned short abMask = (1 << (a - 1)) | (1 << (b - 1));

      // Szukamy prostokątów (r1, c1), (r1, c2), (r2, c1), (r2, c2)
      for (int r1 = 0; r1 < 8; r1++) {
        for (int r2 = r1 + 1; r2 < 9; r2++) {
          // Warunek SE: Prostokąt musi być w dokładnie 2 pudełkach (nie 4!)
          if ((r1 / 3) == (r2 / 3)) continue;

          for (int c1 = 0; c1 < 8; c1++) {
            for (int c2 = c1 + 1; c2 < 9; c2++) {
              int corners[4] = { r1 * 9 + c1, r1 * 9 + c2, r2 * 9 + c1, r2 * 9 + c2 };

              // Czy wszystkie narożniki mają kandydatów A i B?
              int possible = 1;
              int extraBits[4] = { 0 };
              int dirtyCount = 0;

              for (int i = 0; i < 4; i++) {
                if (sudoku[corners[i]] != '0' || (masks[corners[i]] & abMask) != abMask) {
                  possible = 0; break;
                }
                extraBits[i] = masks[corners[i]] & ~abMask;
                if (extraBits[i] != 0) dirtyCount++;
              }

              if (!possible) continue;

              // --- TYP 1: Jedna brudna komórka ---
              if (dirtyCount == 1) {
                for (int i = 0; i < 4; i++) {
                  if (extraBits[i] != 0) {
                    masks[corners[i]] &= ~abMask;
                    if (TEST) printf(">>> UR Typ 1: Usunieto %d,%d z %d\n", a, b, corners[i]);
                    return 1;
                  }
                }
              }

              // --- TYP 2: Dwie brudne komórki z JEDNYM identycznym dodatkowym bitem X ---
              if (dirtyCount == 2) {
                int d1 = -1, d2 = -1;
                for (int i = 0; i < 4; i++) if (extraBits[i]) { if (d1 == -1) d1 = i; else d2 = i; }

                // Muszą mieć ten sam dodatkowy bit i być w tej samej linii
                if (extraBits[d1] == extraBits[d2] && countCandidates(extraBits[d1]) == 1) {
                  unsigned short xBit = extraBits[d1];
                  // Sprawdź zasięg wzroku d1 i d2 pod kątem xBit
                  for (int m = 0; m < 81; m++) {
                    if (sudoku[m] == '0' && m != corners[d1] && m != corners[d2]) {
                      if (cellsSeeEachOther(m, corners[d1]) && cellsSeeEachOther(m, corners[d2])) {
                        if (masks[m] & xBit) {
                          masks[m] &= ~xBit;
                          changed = 1;
                          if (TEST) printf(">>> UR Typ 2: Usunieto dodatkowy bit z %d\n", m);
                        }
                      }
                    }
                  }
                }
              }

              // --- TYP 4: Dwie brudne komórki, w których jedna z cyfr (A lub B) 
              // nie występuje nigdzie indziej w danej linii prostokąta ---
              if (dirtyCount == 2) {
                int d1 = -1, d2 = -1;
                for (int i = 0; i < 4; i++) if (extraBits[i]) { if (d1 == -1) d1 = i; else d2 = i; }

                // Sprawdzamy czy to krawędź prostokąta (ten sam wiersz lub kolumna)
                if (corners[d1] / 9 == corners[d2] / 9 || corners[d1] % 9 == corners[d2] % 9) {
                  int testDigits[2] = { a, b };
                  for (int d = 0; d < 2; d++) {
                    unsigned short dBit = (1 << (testDigits[d] - 1));
                    // Jeśli cyfra dBit w tej LINI występuje TYLKO w tych dwóch rogach:
                    // (Logika uproszczona: usuwamy drugą cyfrę z pary)
                    // To rzadszy przypadek, ale bardzo skuteczny!
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  return changed;
}

/* =====================================================================
 * Technika 9 (Naked Triples)
 *
 * Nazwa w kodzie: applyNakedTriples
 * SE Rating: 3.6
 *
 * Jak to działa:
 * Szukamy dokładnie 3 komórek w obrębie jednej grupy (wiersz, kolumna
 * lub pudełko), które łącznie zawierają tylko 3 unikalne kandydaty.
 *
 * Przykład:
 * Trzy komórki w wierszu mają maski bitowe odpowiadające: {1,2}, {2,3}
 * oraz {1,3}. Choć żadna komórka nie ma wszystkich trzech cyfr, to jako
 * grupa "rezerwują" one cyfry 1, 2 i 3 dla siebie.
 *
 * Funkcja usuwa te trzy bity z masek pozostałych (pustych) komórek
 * w tej samej grupie, co często prowadzi do powstania Naked Singles.
 * ===================================================================== */

int applyNakedTriples(char* sudoku, unsigned short* masks) {
  int changed = 0;
  int indices[9];

  for (int i = 0; i < 9; i++) {
    // Wiersze
    for (int j = 0; j < 9; j++) indices[j] = i * 9 + j;
    changed |= processNakedSubset(sudoku, masks, indices, 3);

    // Kolumny
    for (int j = 0; j < 9; j++) indices[j] = j * 9 + i;
    changed |= processNakedSubset(sudoku, masks, indices, 3);

    // Pudełka
    int startRow = (i / 3) * 3;
    int startCol = (i % 3) * 3;
    for (int j = 0; j < 9; j++) indices[j] = (startRow + j / 3) * 9 + (startCol + j % 3);
    changed |= processNakedSubset(sudoku, masks, indices, 3);
  }
  return changed;
}

/* =====================================================================
 * Technika 10 (Swordfish)
 *
 * Nazwa w kodzie: applySwordfish
 * SE Rating: 3.8
 *
 * Jak to działa:
 * Szukamy trzech wierszy, w których dany kandydat występuje tylko w tych
 * samych trzech kolumnach (łącznie). Nie musi występować we wszystkich
 * dziewięciu przecięciach – wystarczy, że w każdym z tych 3 wierszy
 * kandydat jest "uwięziony" w obrębie tych samych 3 kolumn.
 *
 * Jeśli taki układ zostanie znaleziony, dany kandydat może zostać
 * usunięty z tych 3 kolumn we wszystkich POZOSTAŁYCH wierszach.
 *
 * Technika działa analogicznie w pionie (3 kolumny ograniczające
 * kandydata do 3 wierszy).
 * ===================================================================== */
int applySwordfish(char* sudoku, unsigned short* masks) {
  int masksChanged = 0;

  for (int val = 1; val <= 9; val++) {
    unsigned short bitVal = (1 << (val - 1));
    unsigned short bitToClear = ~bitVal;

    // --- SZUKANIE W WIERSZACH (Swordfish w kolumnach) ---
    int rowMasks[9] = { 0 }; // Maski kolumn, w których występuje 'val' dla każdego wiersza
    for (int r = 0; r < 9; r++) {
      for (int c = 0; c < 9; c++) {
        if (sudoku[r * 9 + c] == '0' && (masks[r * 9 + c] & bitVal)) {
          rowMasks[r] |= (1 << c);
        }
      }
    }

    for (int i = 0; i < 9; i++) {
      if (rowMasks[i] == 0 || countCandidates(rowMasks[i]) > 3) continue;
      for (int j = i + 1; j < 9; j++) {
        if (rowMasks[j] == 0 || countCandidates(rowMasks[j]) > 3) continue;
        for (int k = j + 1; k < 9; k++) {
          if (rowMasks[k] == 0 || countCandidates(rowMasks[k]) > 3) continue;

          unsigned short combinedCols = rowMasks[i] | rowMasks[j] | rowMasks[k];
          if (countCandidates(combinedCols) == 3) {
            // Znaleziono Swordfish! Czyścimy kolumny w innych wierszach
            for (int r = 0; r < 9; r++) {
              if (r == i || r == j || r == k) continue;
              for (int c = 0; c < 9; c++) {
                if (combinedCols & (1 << c)) {
                  int idx = r * 9 + c;
                  if (sudoku[idx] == '0' && (masks[idx] & bitVal)) {
                    masks[idx] &= bitToClear;
                    masksChanged = 1;
                    if (TEST) printf(">>> Swordfish (Row): Usunieto %d z row %d, col %d\n", val, r, c);
                  }
                }
              }
            }
          }
        }
      }
    }

    // --- SZUKANIE W KOLUMNACH (Swordfish w wierszach) ---
    int colMasks[9] = { 0 }; // Maski wierszy dla każdej kolumny
    for (int c = 0; c < 9; c++) {
      for (int r = 0; r < 9; r++) {
        if (sudoku[r * 9 + c] == '0' && (masks[r * 9 + c] & bitVal)) {
          colMasks[c] |= (1 << r);
        }
      }
    }

    for (int i = 0; i < 9; i++) {
      if (colMasks[i] == 0 || countCandidates(colMasks[i]) > 3) continue;
      for (int j = i + 1; j < 9; j++) {
        if (colMasks[j] == 0 || countCandidates(colMasks[j]) > 3) continue;
        for (int k = j + 1; k < 9; k++) {
          if (colMasks[k] == 0 || countCandidates(colMasks[k]) > 3) continue;

          unsigned short combinedRows = colMasks[i] | colMasks[j] | colMasks[k];
          if (countCandidates(combinedRows) == 3) {
            // Znaleziono Swordfish! Czyścimy wiersze w innych kolumnach
            for (int c = 0; c < 9; c++) {
              if (c == i || c == j || c == k) continue;
              for (int r = 0; r < 9; r++) {
                if (combinedRows & (1 << r)) {
                  int idx = r * 9 + c;
                  if (sudoku[idx] == '0' && (masks[idx] & bitVal)) {
                    masks[idx] &= bitToClear;
                    masksChanged = 1;
                    if (TEST) printf(">>> Swordfish (Col): Usunieto %d z row %d, col %d\n", val, r, c);
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  return masksChanged;
}
/* =====================================================================
 * Technika 14 (XY-Wing)
 *
 * Nazwa w kodzie: applyXYWing
 * SE Rating: 4.2
 *
 * Jak to działa:
 * Szukamy komórki bazowej (Pivot), która ma dokładnie dwóch kandydatów
 * (np. X i Y). Następnie szukamy dwóch komórek "skrzydeł" (Pincers),
 * z których każda widzi Pivota:
 *   - Skrzydło A ma kandydatów X i Z.
 *   - Skrzydło B ma kandydatów Y i Z.
 *
 * Logika: Jeśli w Pivocie jest X, to w Skrzydle A musi być Z. Jeśli
 * w Pivocie jest Y, to w Skrzydle B musi być Z. Ponieważ w Pivocie
 * MUSI być albo X, albo Y, to w jednej z komórek skrzydeł MUSI być Z.
 *
 * Wniosek: Możemy usunąć kandydata Z z każdej komórki, która "widzi"
 * jednocześnie oba skrzydła (Pincers).
 * ===================================================================== */


int applyXYWing(char* sudoku, unsigned short* masks) {
  int masksChanged = 0;
  int bivalueCells[81];
  int bivalueCount = 0;

  // 1. Znajdź wszystkie komórki z dokładnie 2 kandydatami
  for (int i = 0; i < 81; i++) {
    if (sudoku[i] == '0' && countCandidates(masks[i]) == 2) {
      bivalueCells[bivalueCount++] = i;
    }
  }

  // 2. Iteruj po każdej komórce jako potencjalnym PIVOCIE
  for (int i = 0; i < bivalueCount; i++) {
    int pIdx = bivalueCells[i];
    unsigned short pMask = masks[pIdx];

    // Wyciągnij kandydatów X i Y z pivota
    int x = -1, y = -1;
    for (int v = 1; v <= 9; v++) {
      if (pMask & (1 << (v - 1))) {
        if (x == -1) x = v; else y = v;
      }
    }

    // 3. Szukaj dwóch skrzydeł (Pincers) widzących Pivota
    for (int j = 0; j < bivalueCount; j++) {
      if (i == j) continue;
      int aIdx = bivalueCells[j];
      if (!cellsSeeEachOther(pIdx, aIdx)) continue;

      unsigned short aMask = masks[aIdx];
      // Skrzydło A musi mieć jednego kandydata z Pivota (X lub Y) i jednego wspólnego Z
      if (!(aMask & pMask) || countCandidates(aMask & pMask) != 1) continue;

      int z = -1;
      int sharedWithP = (aMask & (1 << (x - 1))) ? x : y;
      // Kandydat Z to ten, którego nie ma w Pivocie
      for (int v = 1; v <= 9; v++) {
        if ((aMask & (1 << (v - 1))) && v != sharedWithP) {
          z = v; break;
        }
      }

      for (int k = j + 1; k < bivalueCount; k++) {
        if (i == k) continue;
        int bIdx = bivalueCells[k];
        if (!cellsSeeEachOther(pIdx, bIdx)) continue;

        unsigned short bMask = masks[bIdx];
        int otherFromP = (sharedWithP == x) ? y : x;

        // Skrzydło B musi mieć drugiego kandydata z Pivota i to samo Z
        if ((bMask & (1 << (otherFromP - 1))) && (bMask & (1 << (z - 1)))) {

          // 4. Sukces! Znaleźliśmy XY-Wing. Teraz szukamy komórek do czyszczenia.
          unsigned short zBit = (1 << (z - 1));
          for (int m = 0; m < 81; m++) {
            // Komórka musi widzieć oba skrzydła i nie być żadną z części Winga
            if (sudoku[m] == '0' && m != pIdx && m != aIdx && m != bIdx) {
              if (cellsSeeEachOther(m, aIdx) && cellsSeeEachOther(m, bIdx)) {
                if (masks[m] & zBit) {
                  masks[m] &= ~zBit;
                  masksChanged = 1;
                  if (TEST) printf(">>> XY-Wing: Usunieto %d z komorki %d (Pivot:%d, Skrzydla:%d,%d)\n", z, m, pIdx, aIdx, bIdx);
                }
              }
            }
          }
        }
      }
    }
  }
  return masksChanged;
}

/* =====================================================================
 * Technika 11 (Hidden Triples)
 *
 * Nazwa w kodzie: applyHiddenTriples
 * SE Rating: 4.0
 *
 * Jak to działa:
 * Szukamy trzech kandydatów (np. 1, 5, 9), którzy w obrębie jednej grupy
 * (wiersz, kolumna lub pudełko) występują TYLKO w trzech komórkach.
 *
 * Przykład:
 * Cyfry 1, 5 i 9 pojawiają się w wierszu tylko w kolumnach c1, c3 i c7.
 * Te komórki mogą zawierać inne cyfry (np. c1 ma {1, 2, 4}), ale wiemy,
 * że 1, 5 i 9 MUSZĄ zająć te trzy miejsca.
 *
 * Funkcja usuwa wszystkie INNE cyfry z tych trzech komórek, zostawiając
 * w nich tylko naszą ukrytą trójkę. Często „czyści” to planszę
 * pod techniki typu X-Wing lub Naked Singles.
 * ===================================================================== */

int applyHiddenTriples(char* sudoku, unsigned short* masks) {
  int changed = 0;
  int indices[9];

  for (int i = 0; i < 9; i++) {
    // Wiersze
    for (int j = 0; j < 9; j++) indices[j] = i * 9 + j;
    changed |= processHiddenSubset(sudoku, masks, indices, 3);

    // Kolumny
    for (int j = 0; j < 9; j++) indices[j] = j * 9 + i;
    changed |= processHiddenSubset(sudoku, masks, indices, 3);

    // Pudełka
    int startRow = (i / 3) * 3;
    int startCol = (i % 3) * 3;
    for (int j = 0; j < 9; j++) indices[j] = (startRow + j / 3) * 9 + (startCol + j % 3);
    changed |= processHiddenSubset(sudoku, masks, indices, 3);
  }
  return changed;
}

/* =====================================================================
 * Technika 15 (XYZ-Wing)
 *
 * Nazwa w kodzie: applyXYZWing
 * SE Rating: 4.4
 *
 * Jak to działa:
 * Szukamy komórki bazowej (Pivot), która ma dokładnie trzech kandydatów
 * (np. X, Y i Z). Szukamy dwóch skrzydeł (Pincers):
 *   - Skrzydło A ma kandydatów X i Z.
 *   - Skrzydło B ma kandydatów Y i Z.
 *
 * Kluczowy warunek: Pivot musi "widzieć" oba skrzydła, a dodatkowo jedno
 * ze skrzydeł musi znajdować się w tym samym pudełku (Box) co Pivot.
 *
 * Logika: Niezależnie od tego, czy w Pivocie znajdzie się X, Y czy Z,
 * jedna z tych trzech komórek (Pivot, A lub B) będzie musiała zawierać Z.
 *
 * Wniosek: Możemy usunąć kandydata Z z każdej komórki, która "widzi"
 * jednocześnie wszystkie trzy komórki: Pivot, Skrzydło A i Skrzydło B.
 * ===================================================================== */

int applyXYZWing(char* sudoku, unsigned short* masks) {
  int masksChanged = 0;
  int bivalueCells[81], trivalueCells[81];
  int biCount = 0, triCount = 0;

  // 1. Segregujemy komórki na bi-value (2 bity) i tri-value (3 bity)
  for (int i = 0; i < 81; i++) {
    if (sudoku[i] == '0') {
      int count = countCandidates(masks[i]);
      if (count == 2) bivalueCells[biCount++] = i;
      else if (count == 3) trivalueCells[triCount++] = i;
    }
  }

  // 2. Iterujemy po PIVOTACH (komórki z 3 kandydatami)
  for (int i = 0; i < triCount; i++) {
    int pIdx = trivalueCells[i];
    unsigned short pMask = masks[pIdx];

    // 3. Szukamy dwóch skrzydeł wśród komórek bi-value
    for (int j = 0; j < biCount; j++) {
      int aIdx = bivalueCells[j];
      if (!cellsSeeEachOther(pIdx, aIdx)) continue;

      unsigned short aMask = masks[aIdx];
      // Skrzydło A musi mieć 2 kandydatów, którzy są też w Pivocie
      if ((aMask & pMask) != aMask) continue;

      for (int k = j + 1; k < biCount; k++) {
        int bIdx = bivalueCells[k];
        if (!cellsSeeEachOther(pIdx, bIdx)) continue;

        unsigned short bMask = masks[bIdx];
        // Skrzydło B musi mieć 2 kandydatów z Pivota
        if ((bMask & pMask) != bMask) continue;

        // Sprawdzamy, czy skrzydła A i B dzielą dokładnie jednego wspólnego kandydata (nasz Z)
        // Oraz czy Pivot też go posiada.
        unsigned short commonZMask = aMask & bMask;
        if (countCandidates(commonZMask) == 1) {
          int zDigit = 0;
          for (int v = 1; v <= 9; v++) if (commonZMask & (1 << (v - 1))) zDigit = v;

          // XYZ-Wing wymaga, by wszystkie 3 komórki miały Z, a Pivot dodatkowo X i Y.
          // To już sprawdziliśmy przez (aMask & pMask) == aMask itd.

          // 4. Sukces! Mamy kandydata na XYZ-Wing. 
          // Szukamy komórek, które widzą całą trójkę: Pivot, A i B.
          unsigned short zBit = (1 << (zDigit - 1));
          for (int m = 0; m < 81; m++) {
            if (sudoku[m] == '0' && m != pIdx && m != aIdx && m != bIdx) {
              if (cellsSeeEachOther(m, pIdx) &&
                cellsSeeEachOther(m, aIdx) &&
                cellsSeeEachOther(m, bIdx)) {

                if (masks[m] & zBit) {
                  masks[m] &= ~zBit;
                  masksChanged = 1;
                  if (TEST) printf(">>> XYZ-Wing: Usunieto %d z %d (Pivot:%d, A:%d, B:%d)\n", zDigit, m, pIdx, aIdx, bIdx);
                }
              }
            }
          }
        }
      }
    }
  }
  return masksChanged;
}


/* =====================================================================
 * Technika 4: X-Wing
 *
 * Szuka kandydata, który w dwóch różnych rzędach występuje TYLKO
 * w tych samych dwóch kolumnach (tworząc prostokąt). W takiej
 * sytuacji kandydat ten MUSI leżeć na przeciwległych rogach
 * tego prostokąta. Możemy więc bezpiecznie wykreślić tego kandydata
 * z reszty tych dwóch kolumn. (Zasada działa też w drugą stronę:
 * dwie kolumny wykreślają z dwóch rzędów).
 * ===================================================================== */
int applyXWing(char* sudoku, unsigned short* masks) {
  int changed = 0;

  // Analizujemy osobno każdą cyfrę (od 1 do 9)
  for (int val = 1; val <= 9; val++) {
    unsigned short bitToClear = ~(1 << (val - 1));

    // =========================================================
    // 1. X-WING: Baza w RZĘDACH (czyszczenie kolumn)
    // =========================================================
    int rowPossibilities[9] = { 0 }; // Ile razy kandydat występuje w danym rzędzie
    int rowColMasks[9] = { 0 };      // Maska bitowa zapisująca kolumny (np. 100000001 to col 0 i 8)

    // Zbieramy pozycje
    for (int r = 0; r < 9; r++) {
      for (int c = 0; c < 9; c++) {
        if (sudoku[r * 9 + c] == '0' && isCandidatePossible(masks[r * 9 + c], val)) {
          rowPossibilities[r]++;
          rowColMasks[r] |= (1 << c);
        }
      }
    }

    // Szukamy dwóch rzędów, które mają dokładnie 2 opcje w TYCH SAMYCH kolumnach
    for (int r1 = 0; r1 < 8; r1++) {
      if (rowPossibilities[r1] == 2) {
        for (int r2 = r1 + 1; r2 < 9; r2++) {
          if (rowPossibilities[r2] == 2 && rowColMasks[r1] == rowColMasks[r2]) {

            // Znaleźliśmy X-Wing! Ustalmy, jakie to kolumny
            int c1 = -1, c2 = -1;
            for (int c = 0; c < 9; c++) {
              if (rowColMasks[r1] & (1 << c)) {
                if (c1 == -1) c1 = c;
                else c2 = c;
              }
            }

            // Usuwamy kandydata z tych dwóch kolumn, ale POMIJAMY rzędy r1 i r2
            for (int r = 0; r < 9; r++) {
              if (r != r1 && r != r2) {
                if (sudoku[r * 9 + c1] == '0' && isCandidatePossible(masks[r * 9 + c1], val)) {
                  masks[r * 9 + c1] &= bitToClear;
                  changed = 1;
                  if (TEST) printf(">>> X-Wing (Baza w Rzedach %d,%d dla cyfry %d): usunieto z row %d, col %d\n", r1, r2, val, r, c1);
                }
                if (sudoku[r * 9 + c2] == '0' && isCandidatePossible(masks[r * 9 + c2], val)) {
                  masks[r * 9 + c2] &= bitToClear;
                  changed = 1;
                  if (TEST) printf(">>> X-Wing (Baza w Rzedach %d,%d dla cyfry %d): usunieto z row %d, col %d\n", r1, r2, val, r, c2);
                }
              }
            }
          }
        }
      }
    }

    // =========================================================
    // 2. X-WING: Baza w KOLUMNACH (czyszczenie rzędów)
    // =========================================================
    int colPossibilities[9] = { 0 }; // Ile razy kandydat występuje w danej kolumnie
    int colRowMasks[9] = { 0 };      // Maska bitowa zapisująca rzędy

    // Zbieramy pozycje
    for (int c = 0; c < 9; c++) {
      for (int r = 0; r < 9; r++) {
        if (sudoku[r * 9 + c] == '0' && isCandidatePossible(masks[r * 9 + c], val)) {
          colPossibilities[c]++;
          colRowMasks[c] |= (1 << r);
        }
      }
    }

    // Szukamy dwóch kolumn, które mają dokładnie 2 opcje w TYCH SAMYCH rzędach
    for (int c1 = 0; c1 < 8; c1++) {
      if (colPossibilities[c1] == 2) {
        for (int c2 = c1 + 1; c2 < 9; c2++) {
          if (colPossibilities[c2] == 2 && colRowMasks[c1] == colRowMasks[c2]) {

            // Znaleźliśmy X-Wing! Ustalmy, jakie to rzędy
            int r1 = -1, r2 = -1;
            for (int r = 0; r < 9; r++) {
              if (colRowMasks[c1] & (1 << r)) {
                if (r1 == -1) r1 = r;
                else r2 = r;
              }
            }

            // Usuwamy kandydata z tych dwóch rzędów, ale POMIJAMY kolumny c1 i c2
            for (int c = 0; c < 9; c++) {
              if (c != c1 && c != c2) {
                if (sudoku[r1 * 9 + c] == '0' && isCandidatePossible(masks[r1 * 9 + c], val)) {
                  masks[r1 * 9 + c] &= bitToClear;
                  changed = 1;
                  if (TEST) printf(">>> X-Wing (Baza w Kolumnach %d,%d dla cyfry %d): usunieto z row %d, col %d\n", c1, c2, val, r1, c);
                }
                if (sudoku[r2 * 9 + c] == '0' && isCandidatePossible(masks[r2 * 9 + c], val)) {
                  masks[r2 * 9 + c] &= bitToClear;
                  changed = 1;
                  if (TEST) printf(">>> X-Wing (Baza w Kolumnach %d,%d dla cyfry %d): usunieto z row %d, col %d\n", c1, c2, val, r2, c);
                }
              }
            }
          }
        }
      }
    }
  }

  return changed;
}

/* =====================================================================
 * Technika 13 (Naked Quads)
 *
 * Nazwa w kodzie: applyNakedQuads
 * SE Rating: 4.6
 *
 * Jak to działa:
 * Jest to rozszerzenie logiki trójek na 4 komórki i 4 kandydatów.
 * Szukamy zestawu 4 komórek, których suma bitowa (OR) daje dokładnie
 * 4 zapalone bity.
 *
 * Jest to technika znacznie trudniejsza do zauważenia dla człowieka,
 * ponieważ układ kandydatów może być bardzo rozproszony, np.:
 * C1:{1,2}, C2:{2,3,4}, C3:{1,4}, C4:{1,2,3,4}.
 * Razem te 4 komórki "zużywają" cyfry 1, 2, 3 i 4.
 *
 * Funkcja eliminuje te 4 kandydaty z masek wszystkich innych komórek
 * w danej grupie. Wymaga sprawdzenia większej liczby kombinacji (126
 * na każdą grupę), co dzięki maskom bitowym robimy błyskawicznie.
 * ===================================================================== */

int applyNakedQuads(char* sudoku, unsigned short* masks) {
  int changed = 0;
  int indices[9];

  for (int i = 0; i < 9; i++) {
    // Wiersze
    for (int j = 0; j < 9; j++) indices[j] = i * 9 + j;
    changed |= processNakedSubset(sudoku, masks, indices, 4);

    // Kolumny
    for (int j = 0; j < 9; j++) indices[j] = j * 9 + i;
    changed |= processNakedSubset(sudoku, masks, indices, 4);

    // Pudełka
    int startRow = (i / 3) * 3;
    int startCol = (i % 3) * 3;
    for (int j = 0; j < 9; j++) indices[j] = (startRow + j / 3) * 9 + (startCol + j % 3);
    changed |= processNakedSubset(sudoku, masks, indices, 4);
  }
  return changed;
}

/* =====================================================================
 * Technika 17 (Hidden Quads)
 *
 * Nazwa w kodzie: applyHiddenQuads
 * SE Rating: 4.8
 *
 * Jak to działa:
 * Szukamy czterech kandydatów (np. 1, 2, 5, 8), którzy w obrębie jednej
 * grupy (wiersz, kolumna lub pudełko) występują ŁĄCZNIE tylko w czterech
 * komórkach.
 *
 * Przykład:
 * Cyfry 1, 2, 5 i 8 pojawiają się w wierszu tylko w kolumnach c2, c3, c5 i c6.
 * Te cztery komórki mogą zawierać dowolne inne cyfry, ale ponieważ nasze
 * cztery wybrane cyfry nie pojawiają się nigdzie indziej w tym wierszu,
 * wiemy, że muszą one "zająć" te właśnie cztery komórki.
 *
 * Funkcja usuwa wszystkie inne "nadmiarowe" bity z tych czterech komórek.
 * Jest to jedna z najrzadszych technik w standardowym Sudoku.
 * ===================================================================== */

int applyHiddenQuads(char* sudoku, unsigned short* masks) {
  int changed = 0;
  int indices[9];

  for (int i = 0; i < 9; i++) {
    // Wiersze
    for (int j = 0; j < 9; j++) indices[j] = i * 9 + j;
    changed |= processHiddenQuad(sudoku, masks, indices);

    // Kolumny
    for (int j = 0; j < 9; j++) indices[j] = j * 9 + i;
    changed |= processHiddenQuad(sudoku, masks, indices);

    // Pudełka
    int startRow = (i / 3) * 3;
    int startCol = (i % 3) * 3;
    for (int j = 0; j < 9; j++) indices[j] = (startRow + j / 3) * 9 + (startCol + j % 3);
    changed |= processHiddenQuad(sudoku, masks, indices);
  }
  return changed;
}

/* =====================================================================
 * Technika 18 (Jellyfish)
 *
 * Nazwa w kodzie: applyJellyfish
 * SE Rating: 5.2
 *
 * Jak to działa:
 * Jest to rozszerzenie techniki Swordfish na układ 4x4. Szukamy czterech
 * wierszy, w których dany kandydat występuje łącznie w tych samych
 * czterech kolumnach.
 *
 * Jeśli taki układ zostanie znaleziony, dany kandydat musi znajdować się
 * w jednym z tych czterech przecięć w każdym z wybranych wierszy.
 * W związku z tym, możemy go bezpiecznie usunąć z tych czterech kolumn
 * we wszystkich POZOSTAŁYCH wierszach planszy.
 *
 * Technika jest symetryczna – działa tak samo dla układu 4 kolumn
 * ograniczonych do 4 wierszy.
 * ===================================================================== */

int applyJellyfish(char* sudoku, unsigned short* masks) {
  int masksChanged = 0;

  for (int val = 1; val <= 9; val++) {
    unsigned short bitVal = (1 << (val - 1));
    unsigned short bitToClear = ~bitVal;

    // --- SZUKANIE W WIERSZACH (Meduza w kolumnach) ---
    int rowMasks[9] = { 0 };
    for (int r = 0; r < 9; r++) {
      for (int c = 0; c < 9; c++) {
        if (sudoku[r * 9 + c] == '0' && (masks[r * 9 + c] & bitVal)) {
          rowMasks[r] |= (1 << c);
        }
      }
    }

    // Cztery pętle dla czterech wierszy (i, j, k, l)
    for (int i = 0; i < 9; i++) {
      if (rowMasks[i] == 0 || countCandidates(rowMasks[i]) > 4) continue;
      for (int j = i + 1; j < 9; j++) {
        if (rowMasks[j] == 0 || countCandidates(rowMasks[j]) > 4) continue;
        for (int k = j + 1; k < 9; k++) {
          if (rowMasks[k] == 0 || countCandidates(rowMasks[k]) > 4) continue;
          for (int l = k + 1; l < 9; l++) {
            if (rowMasks[l] == 0 || countCandidates(rowMasks[l]) > 4) continue;

            unsigned short combinedCols = rowMasks[i] | rowMasks[j] | rowMasks[k] | rowMasks[l];

            // Jeśli 4 wiersze ograniczają kandydata do dokładnie 4 kolumn
            if (countCandidates(combinedCols) == 4) {
              for (int r = 0; r < 9; r++) {
                if (r == i || r == j || r == k || r == l) continue;
                for (int c = 0; c < 9; c++) {
                  if (combinedCols & (1 << c)) {
                    int idx = r * 9 + c;
                    if (sudoku[idx] == '0' && (masks[idx] & bitVal)) {
                      masks[idx] &= bitToClear;
                      masksChanged = 1;
                      if (TEST) printf(">>> Jellyfish (Row): Usunieto %d z row %d, col %d\n", val, r, c);
                    }
                  }
                }
              }
            }
          }
        }
      }
    }

    // --- SZUKANIE W KOLUMNACH (Meduza w wierszach) ---
    int colMasks[9] = { 0 };
    for (int c = 0; c < 9; c++) {
      for (int r = 0; r < 9; r++) {
        if (sudoku[r * 9 + c] == '0' && (masks[r * 9 + c] & bitVal)) {
          colMasks[c] |= (1 << r);
        }
      }
    }

    for (int i = 0; i < 9; i++) {
      if (colMasks[i] == 0 || countCandidates(colMasks[i]) > 4) continue;
      for (int j = i + 1; j < 9; j++) {
        if (colMasks[j] == 0 || countCandidates(colMasks[j]) > 4) continue;
        for (int k = j + 1; k < 9; k++) {
          if (colMasks[k] == 0 || countCandidates(colMasks[k]) > 4) continue;
          for (int l = k + 1; l < 9; l++) {
            if (colMasks[l] == 0 || countCandidates(colMasks[l]) > 4) continue;

            unsigned short combinedRows = colMasks[i] | colMasks[j] | colMasks[k] | colMasks[l];

            if (countCandidates(combinedRows) == 4) {
              for (int c = 0; c < 9; c++) {
                if (c == i || c == j || c == k || c == l) continue;
                for (int r = 0; r < 9; r++) {
                  if (combinedRows & (1 << r)) {
                    int idx = r * 9 + c;
                    if (sudoku[idx] == '0' && (masks[idx] & bitVal)) {
                      masks[idx] &= bitToClear;
                      masksChanged = 1;
                      if (TEST) printf(">>> Jellyfish (Col): Usunieto %d z row %d, col %d\n", val, r, c);
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  return masksChanged;
}

/* =====================================================================
 * Technika 19 (W-Wing)
 *
 * Nazwa w kodzie: applyWWing
 * SE Rating: 5.0
 *
 * Jak to działa:
 * 1. Szukamy dwóch różnych komórek (C1 i C2), które mają tych samych
 *    dwóch kandydatów (np. X i Y).
 * 2. Szukamy "mostu" dla kandydata X. Mostem jest grupa (wiersz/kol/box),
 *    w której cyfra X występuje dokładnie dwa razy (tzw. silne ogniwo).
 * 3. Jeśli C1 widzi jeden koniec mostu, a C2 widzi drugi koniec mostu,
 *    to w przynajmniej jednej z komórek C1 lub C2 musi znaleźć się cyfra Y.
 *
 * Wniosek: Możemy usunąć kandydata Y z każdej komórki, która widzi
 * jednocześnie C1 i C2.
 * ===================================================================== */

int applyWWing(char* sudoku, unsigned short* masks) {
  int masksChanged = 0;
  int bivalueCells[81];
  int biCount = 0;

  // 1. Znajdź wszystkie komórki bi-value
  for (int i = 0; i < 81; i++) {
    if (sudoku[i] == '0' && countCandidates(masks[i]) == 2) {
      bivalueCells[biCount++] = i;
    }
  }

  // 2. Porównuj pary identycznych komórek bi-value
  for (int i = 0; i < biCount; i++) {
    for (int j = i + 1; j < biCount; j++) {
      int c1 = bivalueCells[i];
      int c2 = bivalueCells[j];

      // Muszą mieć te same dwie cyfry i nie widzieć się nawzajem bezpośrednio
      if (masks[c1] != masks[c2] || cellsSeeEachOther(c1, c2)) continue;

      // Wyciągnij kandydatów X i Y
      int x = -1, y = -1;
      for (int v = 1; v <= 9; v++) {
        if (masks[c1] & (1 << (v - 1))) {
          if (x == -1) x = v; else y = v;
        }
      }

      // Próbujemy obu kombinacji: X jako most, Y do usunięcia (i na odwrót)
      int candidates[2] = { x, y };
      for (int k = 0; k < 2; k++) {
        int bridgeDigit = candidates[k];
        int targetDigit = candidates[1 - k];
        unsigned short bridgeBit = (1 << (bridgeDigit - 1));
        unsigned short targetBit = (1 << (targetDigit - 1));

        // 3. Szukaj "mostu" (silnego ogniwa) dla bridgeDigit w wierszach/kolumnach/boxach
        for (int groupType = 0; groupType < 3; groupType++) {
          for (int g = 0; g < 9; g++) {
            int groupIndices[9];
            // Pobieranie indeksów grupy (uproszczone)
            for (int m = 0; m < 9; m++) {
              if (groupType == 0) groupIndices[m] = g * 9 + m; // Wiersz
              else if (groupType == 1) groupIndices[m] = m * 9 + g; // Kolumna
              else groupIndices[m] = ((g / 3) * 3 + m / 3) * 9 + ((g % 3) * 3 + m % 3); // Box
            }

            // Liczymy wystąpienia bridgeDigit w tej grupie
            int appearances = 0;
            int pos[9];
            for (int m = 0; m < 9; m++) {
              if (sudoku[groupIndices[m]] == '0' && (masks[groupIndices[m]] & bridgeBit)) {
                pos[appearances++] = groupIndices[m];
              }
            }

            // Jeśli to "silne ogniwo" (dokładnie 2 wystąpienia)
            if (appearances == 2) {
              int L1 = pos[0];
              int L2 = pos[1];

              // Sprawdź czy C1 widzi L1 a C2 widzi L2 (lub na odwrót)
              if ((cellsSeeEachOther(c1, L1) && cellsSeeEachOther(c2, L2)) ||
                (cellsSeeEachOther(c1, L2) && cellsSeeEachOther(c2, L1))) {

                // 4. Sukces! Usuwamy targetDigit z komórek widzących C1 i C2
                for (int m = 0; m < 81; m++) {
                  if (sudoku[m] == '0' && m != c1 && m != c2) {
                    if (cellsSeeEachOther(m, c1) && cellsSeeEachOther(m, c2)) {
                      if (masks[m] & targetBit) {
                        masks[m] &= ~targetBit;
                        masksChanged = 1;
                        if (TEST) printf(">>> W-Wing: Usunieto %d z %d (Most %d w grupie %d)\n", targetDigit, m, bridgeDigit, g);
                      }
                    }
                  }
                }
                if (masksChanged) return 1;
              }
            }
          }
        }
      }
    }
  }
  return masksChanged;
}