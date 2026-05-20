import random
from src.SimulatedAnnealing.SimulatedAnnealing import SimulatedAnnealing
from src.Rating.Rater import Rate_sudoku
from src.CSPSolver import CSPSolver # Podpinamy Twojego świetnego Solvera!

class SA_SudokuGenerator(SimulatedAnnealing[list[bool]]):
    def __init__(self, solved_board: str, initialTemperature: float, coolingRate: float, iterations: int):
        # solved_board to string 81 znaków z poprawnym rozwiązaniem
        self.solved_board = solved_board 
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
        # 1. Konwersja i weryfikacja unikalności
        board_ints = self.apply_mask_to_int_list(individual)
        solver = CSPSolver(board_ints)
        
        # Jeśli nie ma dokładnie jednego rozwiązania -> olbrzymia kara
        if not solver.check_unique_solution():
            return 1000.0  

        # 2. Jeśli jest unikalne, sprawdzamy poziom trudności
        puzzle_str = self.apply_mask_to_str(individual)
        rating = Rate_sudoku(puzzle_str)
        
        # 3. Dodatkowy cel: Im mniej podpowiedzi (clues), tym lepiej.
        clue_count = sum(individual)
        
        # SA szuka najmniejszej wartości (dlatego dajemy minus przed ratingiem)
        # Przykład: Rating 4.0 z 30 podpowiedziami da nam (-4.0) + (1.5) = -2.5
        return -rating + (clue_count * 0.05) 

    def change(self, individual: list[bool]) -> list[bool]:
        new_mask = individual[:]
        idx = random.randint(0, 80)
        
        # Odkrywamy zakryte lub zakrywamy odkryte pole
        new_mask[idx] = not new_mask[idx]
        return new_mask