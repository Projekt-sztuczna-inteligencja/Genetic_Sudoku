import random
from src.Rating.CSPSolver import CSPSolver
from src.SudokuBoard import Board


class PuzzleGenerator:
    """Kopacz Dziur: Tworzy zagadki z pełnych plansz używając Weryfikatora CSP."""

    def __init__(self, full_board: list[int]):
        self.full_board = full_board[:]

    def generate_puzzle(self, target_holes: int = 40) -> list[int]:
        """
        Próbuje usunąć 'target_holes' cyfr z planszy.
        Za każdym usunięciem weryfikuje, czy plansza nadal ma dokładnie 1 rozwiązanie.
        """
        puzzle = self.full_board[:]
        indices = list(range(81))
        random.shuffle(indices)

        holes_dug = 0

        for idx in indices:
            if holes_dug >= target_holes:
                break

            backup_val = puzzle[idx]
            puzzle[idx] = 0

            solver = CSPSolver(puzzle)
            if solver.check_unique_solution():
                holes_dug += 1
            else:
                puzzle[idx] = backup_val

        print(f"Wykonano zagadkę. Udało się wyciągnąć {holes_dug} cyfr.")
        return puzzle


if __name__ == "__main__":
    # Pełna, poprawnie ułożona plansza bazowa (np. wygenerowana przez Twoje C/Genetykę)
    perfect_board = [
        8, 1, 9, 7, 5, 3, 2, 4, 6,
        2, 4, 3, 6, 1, 8, 5, 7, 9,
        6, 7, 5, 2, 9, 4, 1, 8, 3,
        1, 5, 7, 9, 3, 6, 8, 2, 4,
        3, 6, 2, 8, 4, 5, 7, 9, 1,
        9, 8, 4, 1, 2, 7, 6, 3, 5,
        5, 2, 1, 4, 7, 9, 3, 6, 8,
        4, 3, 8, 5, 6, 2, 9, 1, 7,
        7, 9, 6, 3, 8, 1, 4, 5, 2
    ]

    print("🛠️ Rozpoczynam rzeźbienie zagadki (target: 45 pustych pól)...")
    generator = PuzzleGenerator(perfect_board)

    # Próbujemy usunąć 45 cyfr (poziom Medium/Hard)
    puzzle = generator.generate_puzzle(target_holes=45)

    print("\nGotowa zagadka Sudoku (idealna i unikalna):")
    print(Board(puzzle))