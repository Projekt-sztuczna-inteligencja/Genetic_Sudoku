import random
import ctypes
import os
from src.SudokuBoard import Board
from src.Genetic.BaseGenetic import Genetic
from src.Genetic.CrossoverMethod import crossoverType, getCrossoverMethod

# 1. Inicjalizacja skompilowanej biblioteki C
current_dir = os.path.dirname(os.path.abspath(__file__))
dll_path = os.path.join(current_dir, "fitness_fast.dll")

# 2. Ładujemy bibliotekę (winmode=0 jest wymagane w nowszym Pythonie na Windowsie)
fast_c_lib = ctypes.CDLL(dll_path, winmode=0)
fast_c_lib.calculate_fitness.argtypes = [
    ctypes.POINTER(ctypes.c_int),
    ctypes.POINTER(ctypes.c_float),
    ctypes.c_int
]
fast_c_lib.calculate_fitness.restype = None


class FastSudoku(Genetic[list[int]]):
    """Główna klasa odpowiedzialna za reprezentację genetyczną i ocenę plansz Sudoku."""

    def __init__(self, populationSize: int, mutationRate: float, generations: int, eliteSize: int,
                 crossoverFunctionName: crossoverType, initial_board: list[int] = None):
        self.nr = 9
        self.crossoverFunc = getCrossoverMethod(crossoverFunctionName)
        self.initial_board = initial_board if initial_board is not None else [0] * 81
        self.locked_position = [val != 0 for val in self.initial_board]

        super().__init__(populationSize, mutationRate, generations, eliteSize, individualType=list[int])

    def createRandomIndividual(self) -> list[int]:
        """Inicjalizuje planszę z przestrzeganiem zamrożonych pozycji (Clues)."""
        board = []
        for row_idx in range(self.nr):
            row_start = row_idx * 9
            original_row = self.initial_board[row_start:row_start + 9]

            missing_digits = [i for i in range(1, 10) if i not in original_row]
            random.shuffle(missing_digits)

            new_row = []
            missing_ptr = 0
            for val in original_row:
                if val != 0:
                    new_row.append(val)
                else:
                    new_row.append(missing_digits[missing_ptr])
                    missing_ptr += 1
            board.extend(new_row)
        return board

    def crossover(self, parent1: list[int], parent2: list[int]) -> list[int]:
        return self.crossoverFunc(parent1, parent2)

    def fitness(self, population: list[list[int]]) -> list[tuple[list[int], float]]:
        """Wysyła płaską listę całej populacji do szybkiej weryfikacji w języku C."""
        num_boards = len(population)
        flat_population = [val for board in population for val in board]

        C_POP_ARRAY = ctypes.c_int * len(flat_population)
        C_SCORE_ARRAY = ctypes.c_float * num_boards

        c_pop = C_POP_ARRAY(*flat_population)
        c_scores = C_SCORE_ARRAY()

        fast_c_lib.calculate_fitness(c_pop, c_scores, num_boards)

        fit = [float(score) for score in c_scores]
        ret = list(zip(population, fit))
        ret.sort(key=lambda x: x[1], reverse=True)

        return ret

    def selectParents(self, population: list[tuple[list[int], float]], nrParents: int) -> list[list[int]]:
        """Selekcja turniejowa z rozmiarem 4."""
        parents: list[list[int]] = []
        for _ in range(nrParents):
            tournament = random.sample(population, 4)
            winner = max(tournament, key=lambda x: x[1])[0]
            parents.append(winner)
        return parents

    def get_detailed_errors(self, individual: list[int]) -> list[int]:
        """Skanuje planszę w poszukiwaniu błędnych indeksów, przygotowując teren dla mutacji ratunkowej."""
        error_indices = set()

        # Sprawdzanie kolumn
        for c in range(9):
            seen = {}
            for r in range(9):
                idx = (r // 3) * 27 + (c // 3) * 9 + (r % 3) * 3 + (c % 3)
                val = individual[idx]
                if val in seen:
                    error_indices.add(idx)
                    error_indices.add(seen[val])
                seen[val] = idx

        # Sprawdzanie wierszy
        for r in range(9):
            seen = {}
            for c in range(9):
                idx = (r // 3) * 27 + (c // 3) * 9 + (r % 3) * 3 + (c % 3)
                val = individual[idx]
                if val in seen:
                    error_indices.add(idx)
                    error_indices.add(seen[val])
                seen[val] = idx

        return list(error_indices)

    def mutate(self, individual: list[int]) -> list[int]:
        """Zarządza strategią mutacji zależnie od liczby błędów."""
        error_idxs = self.get_detailed_errors(individual)

        # STRATEGIA A: "Lekarz polowy" - naprawia planszę siłowo, jeśli błędów jest mało
        if 0 < len(error_idxs) <= 8:
            target_idx = random.choice(error_idxs)
            sq_id = target_idx // 9

            sq_start = sq_id * 9
            unlocked_in_sq = [
                i for i in range(sq_start, sq_start + 9)
                if not self.locked_position[i] and i != target_idx
            ]

            if unlocked_in_sq:
                swap_with = random.choice(unlocked_in_sq)
                individual[target_idx], individual[swap_with] = individual[swap_with], individual[target_idx]
                return individual

        # STRATEGIA B: Standardowa mutacja losowa dla pozostałych przypadków
        if random.random() < self.mutationRate:
            row_idx = random.randint(0, 8)
            row_start = row_idx * 9
            unlocked = [i for i in range(row_start, row_start + 9) if not self.locked_position[i]]
            if len(unlocked) >= 2:
                idx1, idx2 = random.sample(unlocked, 2)
                individual[idx1], individual[idx2] = individual[idx2], individual[idx1]

        return individual

    def localSearch(self, individual: list[int]) -> list[int]:
        """Przeszukiwanie lokalne: generuje wszystkie sąsiedztwa planszy w poszukiwaniu optymalnego ulepszenia."""
        current_best_ind = individual[:]
        res = self.fitness([current_best_ind])
        current_best_score = res[0][1]

        for sq_idx in range(9):
            sq_start = sq_idx * 9
            unlocked = [i for i in range(sq_start, sq_start + 9) if not self.locked_position[i]]

            if len(unlocked) < 2:
                continue

            potential_neighbors = []
            for i in range(len(unlocked)):
                for j in range(i + 1, len(unlocked)):
                    neighbor = current_best_ind[:]
                    idx1, idx2 = unlocked[i], unlocked[j]
                    neighbor[idx1], neighbor[idx2] = neighbor[idx2], neighbor[idx1]
                    potential_neighbors.append(neighbor)

            if not potential_neighbors:
                continue

            scored_neighbors = self.fitness(potential_neighbors)

            if scored_neighbors[0][1] > current_best_score:
                current_best_ind = scored_neighbors[0][0][:]
                current_best_score = scored_neighbors[0][1]

        return current_best_ind

    def printPopulation(self, nr=0):
        """Wypisuje sformatowane statystyki do konsoli."""
        fitness = self.fitness(self.population)
        score = [individual[1] for individual in fitness]

        print(f"\n---- Generation {nr} ----")
        print(f"Best score:   {max(score) * 100:.2f}%")
        print(f"Median score: {sorted(score)[len(score) // 2] * 100:.2f}%")
        print(f"Average score:{sum(score) / len(score) * 100:.2f}%")
        print(f"Worst score:  {min(score) * 100:.2f}%")

        # Wyciągamy poprawną nazwę użytej funkcji
        cross_name = self.crossoverFunc.__name__
        print(
            f"Settings: [Crossover: {cross_name} | Mutation Rate: {self.mutationRate * 100:.0f}% | Elite: {self.eliteSize}]")

        print("\nBest board in this generation:")
        print(Board(fitness[0][0]))