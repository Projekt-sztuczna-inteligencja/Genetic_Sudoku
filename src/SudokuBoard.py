class Board:
    """Klasa walidująca i estetycznie rysująca planszę Sudoku w konsoli (Wersja Klasyczna Wierszowa)."""

    def __init__(self, board_data: list[int]):
        if len(board_data) != 81:
            raise ValueError("Plansza musi mieć dokładnie 81 elementów!")
        self.board_data = board_data

    def get_val(self, row: int, col: int) -> int:
        """Klasyczne mapowanie wierszowe (od lewej do prawej, z góry na dół)."""
        return self.board_data[row * 9 + col]

    def _is_valid(self, row: int, col: int) -> bool:
        """Sprawdza duplikaty w wierszach, kolumnach i kwadratach."""
        value = self.get_val(row, col)

        if value == 0:
            return False

        for c in range(9):
            if c != col and self.get_val(row, c) == value:
                return False

        for r in range(9):
            if r != row and self.get_val(r, col) == value:
                return False

        start_row = (row // 3) * 3
        start_col = (col // 3) * 3
        for r in range(start_row, start_row + 3):
            for c in range(start_col, start_col + 3):
                if (r != row or c != col) and self.get_val(r, c) == value:
                    return False

        return True

    def __str__(self) -> str:
        """Rysuje sformatowaną, kolorową planszę."""
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

                val = self.get_val(r, c)
                display_val = str(val) if val != 0 else '.'

                # Kolorowanie
                if val != 0 and self._is_valid(r, c):
                    row_str.append(f"{GREEN}{display_val}{RESET}")
                else:
                    row_str.append(f"{RED}{display_val}{RESET}")
                    if val != 0: errors += 1

            lines.append(" ".join(row_str))

        if errors > 0:
            lines.append(f"\n{RED}Liczba konfliktów na planszy: {errors}{RESET}")

        return "\n".join(lines) + "\n"