class Board:
    """Klasa odpowiedzialna za walidację logiki Sudoku oraz estetyczne wyświetlanie planszy."""

    def __init__(self, board_data: list[int]):
        if len(board_data) != 81:
            raise ValueError("Plansza musi mieć dokładnie 81 elementów!")
        self.board_data = board_data

    def get_val(self, row: int, col: int) -> int:
        """Mapuje klasyczne współrzędne (wiersz, kolumna) na indeks w liście opartej na kwadratach 3x3."""
        idx = (row // 3) * 27 + (col // 3) * 9 + (row % 3) * 3 + (col % 3)
        return self.board_data[idx]

    def _is_valid(self, row: int, col: int) -> bool:
        """Sprawdza błędy w wierszach i kolumnach. Kwadratów nie sprawdza, bo z definicji są idealne."""
        value = self.get_val(row, col)

        # Puste miejsca traktujemy jako błędne
        if value in (' ', '.', 0, '0'):
            return False

        # 1. Sprawdzanie wiersza (czy jest powtórka cyfry w tym samym rzędzie)
        for c in range(9):
            if c != col and self.get_val(row, c) == value:
                return False

        # 2. Sprawdzanie kolumny (czy jest powtórka cyfry w tej samej kolumnie)
        for r in range(9):
            if r != row and self.get_val(r, col) == value:
                return False

        return True

    def __str__(self) -> str:
        """Formatuje planszę do wyświetlenia w konsoli z użyciem kolorów ANSI."""
        RED = '\033[91m'
        GREEN = '\033[92m'
        RESET = '\033[0m'

        errors = 0
        lines = []

        for r in range(9):
            if r % 3 == 0 and r != 0:
                lines.append("-" * 21)

            row_str = []
            for c in range(9):
                if c % 3 == 0 and c != 0:
                    row_str.append("|")

                val = str(self.get_val(r, c))

                # Kolorowanie poprawnej/niepoprawnej komórki
                if self._is_valid(r, c):
                    row_str.append(f"{GREEN}{val}{RESET}")
                else:
                    row_str.append(f"{RED}{val}{RESET}")
                    errors += 1

            lines.append(" ".join(row_str))

        lines.append(f"\n{RED}Liczba niepoprawnych komórek: {errors}{RESET}")
        if errors == 0:
            lines.append(f"{GREEN}SUDOKU ROZWIĄZANE IDEALNIE!{RESET}")

        return "\n".join(lines) + "\n"