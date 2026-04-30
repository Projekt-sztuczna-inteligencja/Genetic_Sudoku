#include "Techniques.h"
#include <stdio.h>


SudokuMethodRecord methodRecords[] = {
  // === POZIOM BARDZO ŁATWY / ŁATWY ===
  {"Hidden Singles (Boxes)",      findHiddenSinglesInBoxes,       1.2f},
  {"Hidden Singles (Rows/Cols)",  findHiddenSinglesInRowsAndCols, 1.5f},

  // === POZIOM ŚREDNI ===
  {"Naked Singles",               findNakedSingles,               2.3f},
  {"Pointing Pairs",              applyPointingPairs,             2.6f}, // W SE Pointing to zazwyczaj 2.6 

  // === POZIOM TRUDNY / EKSPERT ===
  {"Naked Pairs",                 applyNakedPairs,                3.0f},
  {"X-Wing",                      applyXWing,                     3.2f},
  {"Hidden Pairs",                applyHiddenPairs,               3.4f}  // Dla ludzi Ukryte Pary są trudniejsze niż X-Wing!
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