class CSPSolver:
    """Weryfikator oparty o Problem Spełniania Ograniczeń (CSP) z heurystyką MRV."""

    def __init__(self, board_data: list[int]):
        self.board = board_data[:]
        self.solutions_count = 0

    def get_possible_values(self, index: int) -> set[int]:
        """Zwraca domenę (dozwolone cyfry) dla konkretnego pustego pola."""
        if self.board[index] != 0:
            return set()

        possible = {1, 2, 3, 4, 5, 6, 7, 8, 9}
        row = index // 9
        col = index % 9

        for i in range(9):
            possible.discard(self.board[row * 9 + i])
            possible.discard(self.board[i * 9 + col])

        start_row = (row // 3) * 3
        start_col = (col // 3) * 3
        for r in range(start_row, start_row + 3):
            for c in range(start_col, start_col + 3):
                possible.discard(self.board[r * 9 + c])

        return possible

    def find_empty_mrv(self) -> int:
        """
        HEURYSTYKA MRV (Minimum Remaining Values):
        Szuka pustego pola, które ma najmniejszą liczbę możliwych kandydatów.
        Zwraca indeks tego pola lub -1, jeśli plansza jest pełna.
        """
        min_options = 10
        best_index = -1

        for idx in range(81):
            if self.board[idx] == 0:

                options = self.get_possible_values(idx)
                num_options = len(options)

                if num_options == 0:
                    return -2

                if num_options < min_options:
                    min_options = num_options
                    best_index = idx

                    if num_options == 1:
                        break

        return best_index

    def solve_and_count(self) -> int:
        """
        Rekurencyjny algorytm Backtrackingu z heurystyką MRV.
        Szuka rozwiązań i zatrzymuje się natychmiast, gdy znajdzie więcej niż jedno.
        Zwraca: 0 (brak rozwiązań), 1 (unikalne rozwiązanie), 2 (więcej niż jedno rozwiązanie).
        """
        if self.solutions_count > 1:
            return self.solutions_count

        best_idx = self.find_empty_mrv()

        if best_idx == -1:
            self.solutions_count += 1
            return self.solutions_count

        if best_idx == -2:
            return self.solutions_count

        possible_values = self.get_possible_values(best_idx)

        for val in possible_values:
            self.board[best_idx] = val

            self.solve_and_count()

            # Cofamy nasz strzał (Backtracking)
            self.board[best_idx] = 0

            if self.solutions_count > 1:
                return self.solutions_count

        return self.solutions_count

    def check_unique_solution(self) -> bool:
        """
        Główny interfejs API do weryfikatora.
        Zwraca True, jeśli plansza ma dokładnie JEDNO rozwiązanie.
        """
        self.solutions_count = 0
        self.solve_and_count()
        return self.solutions_count == 1



if __name__ == "__main__":
    print("🧩 Test Weryfikatora CSP...")

    # Przykład A: Idealne Sudoku (Ekstremalnie trudne, ale ma 1 rozwiązanie)
    unique_puzzle = [
        8, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 3, 6, 0, 0, 0, 0, 0,
        0, 7, 0, 0, 9, 0, 2, 0, 0,
        0, 5, 0, 0, 0, 7, 0, 0, 0,
        0, 0, 0, 0, 4, 5, 7, 0, 0,
        0, 0, 0, 1, 0, 0, 0, 3, 0,
        0, 0, 1, 0, 0, 0, 0, 6, 8,
        0, 0, 8, 5, 0, 0, 0, 1, 0,
        0, 9, 0, 0, 0, 0, 4, 0, 0
    ]

    # Przykład B: Zepsute Sudoku (Brak tylko dwóch cyfr, ale można je wpisać na 2 sposoby)
    non_unique_puzzle = [0]*81

    print("\nSprawdzam planszę A (powinna być unikalna):")
    solverA = CSPSolver(unique_puzzle)
    is_unique_A = solverA.check_unique_solution()
    print(f"Wynik: {'✅ UNIKALNA' if is_unique_A else '❌ NIEUNIKALNA'}")

    print("\nSprawdzam planszę B (powinna być nieunikalna):")
    solverB = CSPSolver(non_unique_puzzle)
    is_unique_B = solverB.check_unique_solution()
    print(f"Wynik: {'✅ UNIKALNA' if is_unique_B else '❌ NIEUNIKALNA'}")