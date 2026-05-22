import random
from src.Rating.PerfectRater import get_se_rating
from src.SimulatedAnnealing.SimulatedAnnealing import SimulatedAnnealing
from src.Rating.Rater import Rate_sudoku
from src.CSPSolver import CSPSolver # Podpinamy Twojego świetnego Solvera!

class SA_SudokuGenerator(SimulatedAnnealing[list[bool]]):
    def __init__(self, solved_board: str, initialTemperature: float, coolingRate: float, iterations: int, target_rating: float, use_perfect_rater: bool = False):
        # solved_board to string 81 znaków z poprawnym rozwiązaniem
        self.solved_board = solved_board 
        self.target_rating = target_rating
        self.use_perfect_rater = use_perfect_rater
        super().__init__(initialTemperature, coolingRate, iterations, list)

    def createRandomIndividual(self) -> list[bool]:
        # Zaczynamy od np. 35 odkrytych pól (bardzo łatwe Sudoku)
        mask = [False] * 81
        clues_indices = random.sample(range(81), 35)
        for idx in clues_indices:
            mask[idx] = True
        return mask

    def apply_mask_to_str(self, mask: list[bool]) -> str:
        """Dla Gradera: Zwraca stringa (np. '80003...')"""
        return "".join([self.solved_board[i] if mask[i] else "0" for i in range(81)])

    def apply_mask_to_int_list(self, mask: list[bool]) -> list[int]:
        """Dla CSPSolvera: Zwraca listę intów (np. [8, 0, 0, 0, 3...])"""
        return [int(self.solved_board[i]) if mask[i] else 0 for i in range(81)]

    def cost(self, individual: list[bool]) -> float:
        board_ints = self.apply_mask_to_int_list(individual)
        solver = CSPSolver(board_ints)
        
        if not solver.check_unique_solution():
            return 1000.0  # Olbrzymia kara za brak jednego rozwiązania

        puzzle_str = self.apply_mask_to_str(individual)
        if self.use_perfect_rater:
            rating = get_se_rating(puzzle_str)
        else:
            rating = Rate_sudoku(puzzle_str)
        if rating < 0:
            return 800.0
        
        rating_penalty = 2 ** abs(rating - self.target_rating)
        if rating_penalty == 0:
            self.iterations = 0  # Natychmiast zakończ, jeśli osiągniemy idealny wynik
        if rating_penalty <= 2.0:
            print(f"🎉 Osiągnięto idealny rating! {rating} == {self.target_rating}")
        return rating_penalty - 1

    def change(self, individual: list[bool]) -> list[bool]:
        new_mask = individual[:]
        idx = random.randint(0, 80)
        
        # Odkrywamy zakryte lub zakrywamy odkryte pole
        new_mask[idx] = not new_mask[idx]
        return new_mask