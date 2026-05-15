# C Sudoku Solver & SE Rater

Zaawansowany projekt do rozwiązywania i oceniania trudności plansz Sudoku, napisany w języku C. Wykorzystuje zaawansowane operacje bitowe (maski) do błyskawicznej dedukcji oraz obsługuje szeroki wachlarz technik logicznych – od podstawowych po eksperckie.

## Struktura Projektu

Projekt podzielony jest na dwa główne moduły ułożone w osobnych folderach:

* **SolvingTechniques/**
  * `Techniques.c` / `Techniques.h` - Implementacje technik rozwiązywania Sudoku.
  * `Utils.c` / `Utils.h` - Funkcje pomocnicze, operacje na maskach bitowych, walidacja.
* **Rater/**
  * `Rater.c` / `Rater.h` - Algorytm oceniający trudność Sudoku (SE Rating) na podstawie użytych metod.

## Zaimplementowane Techniki

Biblioteka posiada wbudowaną tablicę wskaźników na funkcje `methodRecords`, która przechowuje metody posortowane od najprostszych do najbardziej zaawansowanych.

| Kategoria        | Technika             | SE Rating |
| :--------------- | :------------------- | :-------- |
| Bardzo Łatwy    | Hidden Single (Box)  | 1.2       |
| Bardzo Łatwy    | Hidden Single (Line) | 1.5       |
| Łatwy / Średni | Naked Single         | 2.3       |
| Łatwy / Średni | Pointing Pairs       | 2.6       |
| Łatwy / Średni | Claiming (Box-Line)  | 2.8       |
| Trudny           | Naked Pair           | 3.0       |
| Trudny           | X-Wing               | 3.2       |
| Trudny           | Hidden Pair          | 3.4       |
| Trudny           | Unique Rectangle     | 3.5       |
| Trudny           | Naked Triple         | 3.6       |
| Trudny           | Swordfish            | 3.8       |
| Ekspert          | Hidden Triple        | 4.0       |
| Ekspert          | XY-Wing              | 4.2       |
| Ekspert          | XYZ-Wing             | 4.4       |
| Ekspert          | Naked Quad           | 4.6       |
| Ekspert          | Hidden Quad          | 4.8       |
| Ekspert          | W-Wing               | 5.0       |
| Ekspert          | Jellyfish            | 5.2       |

---

## Kompilacja do biblioteki dla Pythona

Aby użyć kodu C w Pythonie, należy skompilować go do postaci biblioteki współdzielonej (Shared Library). Proces różni się nieznacznie w zależności od systemu operacyjnego.

Uruchom terminal w głównym katalogu projektu (tym, który zawiera foldery `SolvingTechniques` oraz `Rater`).

**Dla systemów Linux / macOS (.so):**

```bash
gcc -shared -fPIC -O3 -o libsudoku.so SolvingTechniques/Utils.c SolvingTechniques/Techniques.c Rater/Rater.c
```

**Dla systemów Windows (.dll):**

```bash
gcc -shared -O3 -o src/Raing/libsudoku.dll src/Rating/SolvingTechniques/Utils.c src/Rating/SolvingTechniques/Techniques.c src/Rating/Rater.c
```
