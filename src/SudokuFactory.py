import random
from CSPSolver import CSPSolver
from ExpertSystemGrader import ExpertSystemGrader
from SudokuBoard import Board
from Rating.Rater import Rate_sudoku


class SudokuFactory:
    """
    Ostateczna Fabryka Zagadek.
    Rzeźbi zagadki z gwarancją trafienia w punkt wybranego poziomu trudności.
    Zastosowano mechanizm odrzucania ruchów przekraczających cel oraz pętle prób.
    """

    def __init__(self, full_board: list[int]):
        self.full_board = full_board[:]

        self.difficulty_levels = {
            "EASY": 1.5,
            "MEDIUM": 2.5,
            "HARD": 5.0,
            "EVIL": 10.0
        }
        self.difficulty_levels_min = {
            "EASY": 0.0,
            "MEDIUM": 1.5,
            "HARD": 2.5,
            "EVIL": 5.0
        }

    def generate(self, target_difficulty: str = "MEDIUM") -> list[int]:
        target_level = self.difficulty_levels.get(target_difficulty, 2)
        target_level_min = self.difficulty_levels_min.get(target_difficulty, 1.5)  # Domyślnie MEDIUM
        attempts = 0


        while True:
            attempts += 1
            print(f"🏭 Próba {attempts}: Rzeźbienie planszy (Cel: {target_difficulty})...")

            puzzle = self.full_board[:]
            indices = list(range(81))
            random.shuffle(indices)

            holes_dug = 0
            current_level = 1


            for idx in indices:
                backup_val = puzzle[idx]
                puzzle[idx] = 0


                solver = CSPSolver(puzzle)
                if solver.check_unique_solution():

                    # 2. Ocena trudności wykopanej planszy
                    grader = ExpertSystemGrader(puzzle)
                    # report = grader.grade_puzzle()

                    # # Ekstrakcja słowa (EASY/MEDIUM/HARD/EVIL) z raportu Agenta
                    # diff_str = report.split()[0]
                    # level = self.difficulty_levels.get(diff_str, 4)
                    puzzle_str = ''.join(str(x) for x in puzzle)
                    level = Rate_sudoku(puzzle_str)

                    # 3. Zabezpieczenie przed ZBYT TRUDNĄ planszą
                    if level > target_level:
                        # Za mocno! Zakopujemy z powrotem.
                        puzzle[idx] = backup_val
                    else:
                        # Akceptujemy dziurę (jest unikalna i nie przekracza naszego celu)
                        holes_dug += 1
                        current_level = level
                        # Możesz odkomentować poniższego printa, żeby widzieć kopanie na żywo:
                        # print(f"   Dziury: {holes_dug:02d} | Ocena: {current_report}")
                else:
                    # Straciliśmy unikalność - zakopujemy
                    puzzle[idx] = backup_val

            # Po sprawdzeniu wszystkich 81 pól sprawdzamy, gdzie wylądowaliśmy
            if current_level <= target_level and current_level >= target_level_min:
                print(
                    f"\n🎉 SUKCES! W próbie {attempts} wygenerowano idealne {target_difficulty} (Pustych pól: {holes_dug})!")
                return puzzle
            else:
                # Trafiliśmy ZA NISKO (np. skończyły się pola, a plansza to nadal EASY)
                print(f"⚠️ Próba {attempts} zakończona na {current_level}. To za łatwe. Próbuję od nowa...\n")


# --- TEST FABRYKI ---
if __name__ == "__main__":
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

    factory = SudokuFactory(perfect_board)

    # Teraz Fabryka będzie mielić planszę tyle razy, aż trafi IDEALNIE W HARD
    final_puzzle = factory.generate(target_difficulty="EASY")
    print("\nOto Twoje gotowe do druku Sudoku:")
    print(Board(final_puzzle))
    final_puzzle = factory.generate(target_difficulty="MEDIUM")
    print("\nOto Twoje gotowe do druku Sudoku:")
    print(Board(final_puzzle))
    final_puzzle = factory.generate(target_difficulty="HARD")

    print("\nOto Twoje gotowe do druku Sudoku:")
    print(Board(final_puzzle))