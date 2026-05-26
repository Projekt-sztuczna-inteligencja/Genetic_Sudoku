# 🧩 Generator i Weryfikator Zagadek Sudoku

Zaawansowany projekt w języku Python przeznaczony do generowania, walidacji oraz oceny trudności zagadek Sudoku. Aplikacja łączy podejście algorytmiczne (Constraint Satisfaction Problem) z systemem eksperckim symulującym ludzkie techniki dedukcji, co pozwala na tworzenie grywalnych plansz o precyzyjnie określonym poziomie trudności z gwarancją unikalności rozwiązania.

---

## 📁 Struktura Projektu

```text
projekt/
├── .venv/                  # Środowisko wirtualne
├── src/
│   ├── Rating/
│   │   ├── CSPSolver.py           # Silnik CSP z heurystyką MRV (weryfikacja unikalności)
│   │   └── ExpertSystemGrader.py  # System ekspercki oparty na regułach dedukcji (ocena trudności)
│   ├── Simulation/
│   │   ├── PuzzleGenerator.py     # Algorytm drążenia otworów (Kopacz Dziur)
│   │   └── SudokuFactory.py       # Fabryka zagadek celująca w konkretny poziom trudności
│   └── SudokuBoard.py             # Klasa planszy (walidacja, renderowanie ANSI w konsoli)
└── README.md                      # Dokumentacja projektu (ten plik)
```

---

## 🚀 Główne Komponenty i Moduły

### 1. `SudokuBoard.py` — Klasa `Board`

Odpowiada za niskopoziomową reprezentację planszy 9×9 (jako jednowymiarowa lista 81 elementów).

- **Walidacja:** Dynamicznie sprawdza poprawność cyfr w wierszach, kolumnach i podkwadratach 3×3.
- **Estetyczne rysowanie:** Generuje sformatowany widok planszy w terminalu z użyciem kolorów ANSI. Cyfry poprawne podświetlane są na **zielono**, puste miejsca lub konflikty na **czerwono**. Informuje również o łącznej liczbie konfliktów.

### 2. `CSPSolver.py` — Klasa `CSPSolver`

Weryfikator unikalności rozwiązania oparty o **Problem Spełniania Ograniczeń (CSP)**.

- **Algorytm:** Backtracking połączony z heurystyką **MRV (Minimum Remaining Values)**. Algorytm w pierwszej kolejności uzupełnia pola o najmniejszej liczbie dozwolonych cyfr, co drastycznie przyspiesza przeszukiwanie.
- **Weryfikacja unikalności:** Funkcja `solve_and_count()` nie tylko szuka rozwiązania, ale zlicza je. Przerywa działanie natychmiast po znalezieniu drugiego rozwiązania, zwracając: *0 (brak), 1 (unikalne), 2 (wiele rozwiązań)*. Jest to kluczowe narzędzie dla generatora.

### 3. `ExpertSystemGrader.py` — Klasa `ExpertSystemGrader`

System ekspercki oceniający trudność planszy na podstawie **ludzkich technik dedukcji**. Zamiast brutalnej siły, silnik próbuje rozwiązać planszę krok po kroku za pomocą 4 zaimplementowanych reguł:

1. **Naked Single** *(Bardzo łatwa)* — pole ma tylko jedną możliwą cyfrę w swojej domenie.
2. **Hidden Single** *(Łatwa)* — cyfra może wystąpić tylko w jednym polu w obrębie danego wiersza/kolumny/kwadratu.
3. **Naked Pair** *(Średnia)* — dwa pola w tym samym sektorze mają identyczną domenę złożoną z dokładnie dwóch cyfr, co pozwala usunąć je z pozostałych pól sektora.
4. **Pointing Line** *(Trudna)* — kandydaci na daną cyfrę w kwadracie układają się w jedną linię, co pozwala wyeliminować ich z reszty tego wiersza/kolumny.

**System punktacji:** Każda użyta technika ma swoją wagę (od 1 do 40 pkt). Na podstawie sumy punktów oraz konieczności zgadywania, plansza klasyfikowana jest jako `EASY`, `MEDIUM`, `HARD` lub `EVIL`.

### 4. `PuzzleGenerator.py` — Klasa `PuzzleGenerator`

Tzw. *Kopacz Dziur*. Przyjmuje pełną, prawidłowo rozwiązaną planszę i próbuje usunąć z niej określoną liczbę cyfr (`target_holes`).

- Przetwarza pola w losowej kolejności.
- Przed zatwierdzeniem usunięcia cyfry przesyła planszę do `CSPSolver` w celu weryfikacji unikalności. Jeśli unikalność zostanie utracona — cyfra jest przywracana.

### 5. `SudokuFactory.py` — Klasa `SudokuFactory`

Ostateczna fabryka, która gwarantuje wygenerowanie planszy o **dokładnie wybranym poziomie trudności** (`EASY`, `MEDIUM`, `HARD`, `EVIL`).

- Wykorzystuje mechanizm pętli prób (rejection sampling).
- Podczas usuwania cyfr na bieżąco sprawdza poziom trudności za pomocą `ExpertSystemGrader`.
- **Zabezpieczenie przed przestrzeleniem:** Jeśli usunięcie cyfry spowodowałoby, że plansza stanie się trudniejsza niż cel, ruch jest cofany.
- Jeśli algorytm zakończy pracę, a poziom jest zbyt niski, proces generowania startuje od nowa, aż do uzyskania idealnego rezultatu.

---

## 🛠️ Jak Uruchomić Projekt

Projekt nie wymaga instalowania zewnętrznych bibliotek — wykorzystuje wyłącznie wbudowane moduły Pythona (m.in. `random`).

### Uruchomienie fabryki (generowanie zagadki)

```bash
python -m src.Simulation.SudokuFactory
```

W sekcji `__main__` pliku możesz zmienić docelowy poziom trudności:

```python
final_puzzle = factory.generate(target_difficulty="HARD")  # Dostępne: EASY, MEDIUM, HARD, EVIL
```

### Testowanie komponentów

Poszczególne elementy systemu można przetestować niezależnie:

```bash
# Weryfikator CSP
python -m src.Rating.CSPSolver

# Kopacz Dziur
python -m src.Simulation.PuzzleGenerator
```

---

## 📊 Przykład Działania Konsoli

Podczas generowania poziomu `EVIL`, fabryka raportuje postępy w konsoli:

```text
🏭 Próba 1: Rzeźbienie planszy (Cel: EVIL)...
⚠️ Próba 1 zakończona na HARD. To za łatwe. Próbuję od nowa...

🏭 Próba 2: Rzeźbienie planszy (Cel: EVIL)...

🎉 SUKCES! W próbie 2 wygenerowano idealne EVIL (Pustych pól: 52)!

Oto Twoje gotowe do druku Sudoku:
. . . | . . . | . . .
. . 3 | 6 . . | . . .
. 7 . | . 9 . | 2 . .
---------------------
. 5 . | . . 7 | . . .
. . . | . 4 5 | 7 . .
. . . | 1 . . | . 3 .
---------------------
. . 1 | . . . | . 6 8
. . 8 | 5 . . | . 1 .
. 9 . | . . . | 4 . .
```

---

*Projekt przygotowany w ramach nauki struktur algorytmicznych, systemów agentowych / eksperckich oraz problemów klasy CSP.*