# Rozwiązywanie Sudoku za Pomocą Algorytmu Genetycznego

Ten podkatalog (`src/Genetic/`) zawiera pełną implementację **Algorytmu Genetycznego (GA)** dedykowanego automatycznemu rozwiązywaniu łamigłówek Sudoku. Moduł przyjmuje jako wejście niekompletną planszę (z pustymi polami) i poprzez proces ewolucyjny — obejmujący selekcję, krzyżowanie oraz mutacje — dąży do znalezienia w pełni poprawnego rozwiązania, które nie generuje żadnych konfliktów w wierszach, kolumnach ani blokach 3×3.

---

## 📋 Spis treści

1. [Główne Funkcje](#-główne-funkcje)
2. [Architektura Modułu Genetycznego](#-architektura-modułu-genetycznego)
3. [Reprezentacja i Mechanizm Ewolucyjny](#-reprezentacja-i-mechanizm-ewolucyjny)
4. [Optymalizacja Wydajności (Natywny Kod C)](#-optymalizacja-wydajności-natywny-kod-c)
5. [Konfiguracja i Parametry](#-konfiguracja-i-parametry)
6. [Jak Uruchomić Solver](#-jak-uruchomić-solver)

---

## 🚀 Główne Funkcje

- **Ewolucyjne przeszukiwanie przestrzeni stanów:** Efektywne unikanie lokalnych minimów dzięki zróżnicowanej populacji osobników i operatorom stochastycznym.
- **Wymienne operatory krzyżowania:** Elastyczna architektura pozwalająca na testowanie różnych strategii wymiany materiału genetycznego (np. krzyżowanie zorientowane na wiersze lub bloki).
- **Ekstremalne przyspieszenie dzięki C (`ctypes`):** Przeniesienie krytycznej funkcji dopasowania (*fitness function*) do skompilowanej biblioteki C, co pozwala na przetwarzanie tysięcy osobników na sekundę.
- **Abstrakcyjny szkielet GA:** Wykorzystanie silnie typowanej klasy bazowej, co ułatwia ewentualną rozbudowę lub modyfikację parametrów ewolucji.

---

## 🏗️ Architektura Modułu Genetycznego

```text
src/Genetic/
│
├── BaseGenetic.py      # Abstrakcyjna klasa bazowa dla algorytmu genetycznego
├── SudokuEngine.py     # Specjalizacja GA pod problem Sudoku (reprezentacja, fitness)
├── CrossoverMethod.py  # Implementacje różnych operatorów krzyżowania (Crossover)
├── run_solver.py       # Główny skrypt uruchomieniowy i diagnostyczny solvera
│
├── fitness_fast.c      # Kod źródłowy w języku C do szybkiego liczenia konfliktów
└── fitness_fast.dll    # Skompilowana biblioteka dynamiczna (Windows)
```

### Opis komponentów

| Plik | Opis |
|------|------|
| `BaseGenetic.py` | Definiuje ogólny przepływ algorytmu genetycznego (inicjalizacja populacji, pętla pokoleń, selekcja turniejowa/ruletkowa, sukcesja elitarnej części populacji). Nie zawiera kodu specyficznego dla Sudoku — stanowi uniwersalny silnik. |
| `SudokuEngine.py` | Łączy generyczny silnik z domeną Sudoku. Odpowiada za mapowanie tablicy Sudoku na chromosom, blokowanie pól zdefiniowanych jako stałe podpowiedzi (baza startowa) oraz ewaluację osobników. |
| `CrossoverMethod.py` | Zawiera dedykowane techniki krzyżowania. Ponieważ klasyczne krzyżowanie punktowe mogłoby zniszczyć strukturę Sudoku, zaimplementowano tu operatory operujące na całych strukturach (np. zamiana losowych wierszy lub podbloków między rodzicami). |
| `fitness_fast.c` | Kluczowy element optymalizacyjny. Zliczanie duplikatów cyfr w 9 wierszach, 9 kolumnach i 9 blokach w czystym Pythonie stanowiłoby ogromne wąskie gardło (*bottleneck*). Kod w C realizuje to w czasie mikrosekundowym. |

---

## 🧠 Reprezentacja i Mechanizm Ewolucyjny

### Kodowanie Chromosomu

Każdy osobnik (chromosom) reprezentuje jedną instancję planszy Sudoku. Pola początkowe (podpowiedzi) są **zamrożone** — algorytm nie może ich modyfikować. Puste pola są inicjalizowane w sposób losowy lub półlosowy (np. wypełniając każdy blok 3×3 unikalnymi cyframi od 1 do 9, co od razu eliminuje błędy wewnątrz bloków).

### Funkcja Dopasowania (Fitness)

Funkcja dopasowania dąży do **minimalizacji liczby błędów** (konfliktów). Idealny osobnik to taki, którego wartość fitness wynosi `0` (rozwiązanie poprawne). Wzór na liczbę konfliktów sprawdza:

- powtórzenia cyfr w każdym z 9 wierszy,
- powtórzenia cyfr w każdej z 9 kolumn,
- powtórzenia cyfr w każdym z 9 podkwadratów 3×3.

---

## ⚡ Optymalizacja Wydajności (Natywny Kod C)

Jeśli chcesz zmodyfikować lub skompilować funkcję fitness dla swojego systemu, kod źródłowy znajduje się w pliku `fitness_fast.c`.

### Kompilacja (opcjonalna)

```bash
# Linux / macOS (.so)
gcc -shared -o fitness_fast.so -fPIC fitness_fast.c

# Windows (MinGW) (.dll)
gcc -shared -o fitness_fast.dll fitness_fast.c
```

Moduł `SudokuEngine.py` automatycznie wykrywa system operacyjny i ładuje odpowiednie rozszerzenie za pomocą biblioteki `ctypes`.

---

## ⚙️ Konfiguracja i Parametry

Główne hiperparametry algorytmu genetycznego konfiguruje się w pliku `run_solver.py` lub przekazuje bezpośrednio do silnika:

| Parametr | Opis | Sugerowana wartość |
|---|---|---|
| `population_size` | Liczba osobników w populacji żyjących jednocześnie. | `200` – `1000` |
| `mutation_rate` | Prawdopodobieństwo zajścia mutacji u pojedynczego osobnika. | `0.05` – `0.2` |
| `crossover_rate` | Prawdopodobieństwo krzyżowania pary rodziców. | `0.7` – `0.9` |
| `elite_size` | Liczba najlepszych osobników przenoszonych bez zmian do nowego pokolenia. | `5%` – `10%` populacji |
| `generations` | Maksymalna liczba epok (pokoleń) przed przerwaniem algorytmu. | `1000`+ |

---

## 💻 Jak Uruchomić Solver

W celu uruchomienia procesu rozwiązywania Sudoku, wykonaj skrypt `run_solver.py` z poziomu głównego katalogu projektu:

```bash
python -m src.Genetic.run_solver
```

### Przykład integracji w kodzie

```python
from src.Genetic.SudokuEngine import FastSudoku
from src.Genetic.run_solver import transform_to_squares
from src.SudokuBoard import Board

# 1. Przygotowanie planszy wejściowej jako jednowymiarowej listy (0 oznacza puste pole)
sample_puzzle = [
    0, 0, 0, 0, 0, 0, 0, 1, 0,
    4, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 2, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 5, 0, 4, 0, 7,
    0, 0, 8, 0, 0, 0, 3, 0, 0,
    0, 0, 1, 0, 9, 0, 0, 0, 0,
    3, 0, 0, 4, 0, 0, 2, 0, 0,
    0, 5, 0, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 8, 0, 6, 0, 0, 0
]

# Przetworzenie klasycznej planszy na wewnętrzną reprezentację genetyczną (bloki 3x3)
genetic_puzzle = transform_to_squares(sample_puzzle)

# 2. Inicjalizacja szybkiego solvera genetycznego
solver = FastSudoku(
    populationSize=2000,
    mutationRate=0.6,
    generations=100000,
    eliteSize=100,
    crossoverFunctionName="uniform", 
    initial_board=genetic_puzzle
)

# 3. Uruchomienie ewolucji
print("Trwa uruchamianie ewolucyjnego solvera Sudoku...")
rozwiazanie = solver.run()

if rozwiazanie:
    print("🎉 Sukces! Znaleziono rozwiązanie:")
    # Obiekt Board automatycznie waliduje i koloruje planszę przy użyciu print()
    print(Board(rozwiazanie)) 
else:
    print("Nie udało się znaleźć idealnego rozwiązania w zadanej liczbie pokoleń.")
```
