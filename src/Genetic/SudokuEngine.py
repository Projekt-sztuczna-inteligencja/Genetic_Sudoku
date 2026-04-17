import random
import ctypes
import os
from src.SudokuBoard import Board
from src.Genetic.BaseGenetic import Genetic
from src.Genetic.CrossoverMethod import crossoverType, getCrossoverMethod

# 1. Absolutna ścieżka do folderu, w którym leży skompilowana biblioteka
current_dir = os.path.dirname(os.path.abspath(__file__))
dll_path = os.path.join(current_dir, "fitness_fast.dll")

# 2. Ładowanie biblioteki do weryfikacji sprzętowej w języku C
fast_c_lib = ctypes.CDLL(dll_path, winmode=0)
fast_c_lib.calculate_fitness.argtypes = [
    ctypes.POINTER(ctypes.c_int),
    ctypes.POINTER(ctypes.c_float),
    ctypes.c_int
]
fast_c_lib.calculate_fitness.restype = None


class FastSudoku(Genetic[list[int]]):
    """Implementacja algorytmu genetycznego dla Sudoku w ujęciu kwadratowym z API do C."""

    def __init__(self, populationSize: int, mutationRate: float, generations: int, eliteSize: int,
                 crossoverFunctionName: crossoverType):
        self.nr = 9
        self.crossoverFunc = getCrossoverMethod(crossoverFunctionName)
        super().__init__(populationSize, mutationRate, generations, eliteSize, individualType=list[int])

    def createRandomIndividual(self) -> list[int]:
        """Inicjalizuje planszę przez losowe tasowanie cyfr 1-9 dla każdego z 9 kwadratów."""
        board = []
        for _ in range(self.nr):
            row = [i for i in range(1, self.nr + 1)]
            random.shuffle(row)
            board.extend(row)
        return board

    def crossover(self, parent1: list[int], parent2: list[int]) -> list[int]:
        """Krzyżuje rodziców zadaną w konstruktorze metodą z rejestru."""
        return self.crossoverFunc(parent1, parent2)

    def fitness(self, population: list[list[int]]) -> list[tuple[list[int], float]]:
        """Przesyła całą populację do szybkiej weryfikacji poprawności w C."""
        num_boards = len(population)

        # Spłaszczenie populacji do jednowymiarowej listy
        flat_population = [val for board in population for val in board]

        # Konwersja listy na tablice zgodne ze wskaźnikami C
        C_POP_ARRAY = ctypes.c_int * len(flat_population)
        C_SCORE_ARRAY = ctypes.c_float * num_boards

        c_pop = C_POP_ARRAY()
        c_pop[:] = flat_population
        c_scores = C_SCORE_ARRAY()

        # Wywołanie skompilowanego kodu C
        fast_c_lib.calculate_fitness(c_pop, c_scores, num_boards)

        # Zebranie wyników i sortowanie malejąco wg oceny
        fit = [float(score) for score in c_scores]
        ret = list(zip(population, fit))
        ret.sort(key=lambda x: x[1], reverse=True)

        return ret

    def selectParents(self, population: list[tuple[list[int], float]], nrParents: int) -> list[list[int]]:
        """Selekcja turniejowa (wielkość turnieju = 4)."""
        parents: list[list[int]] = []
        for _ in range(nrParents):
            tournament = random.sample(population, 4)
            winner = max(tournament, key=lambda x: x[1])[0]
            parents.append(winner)
        return parents

    def mutate(self, individual: list[int]) -> list[int]:
        """Mutacja typu 'Snajper' - rotuje 2 losowe komórki tylko w 1 z 9 kwadratów."""
        if random.random() < self.mutationRate:
            i = random.randint(0, 8)
            size = 9
            idx1 = random.randint(i * size, (i + 1) * size - 1)
            idx2 = random.randint(i * size, (i + 1) * size - 1)
            individual[idx1], individual[idx2] = individual[idx2], individual[idx1]
        return individual

    def printPopulation(self, nr=0):
        """Wypisuje sformatowane statystyki danej generacji do konsoli."""
        fitness_scores = self.fitness(self.population)
        scores = [individual[1] for individual in fitness_scores]

        print(f"\n---- Generation {nr} ----")
        print(f"Best score:   {max(scores) * 100:.2f}%")
        print(f"Median score: {sorted(scores)[len(scores) // 2] * 100:.2f}%")
        print(f"Average score:{sum(scores) / len(scores) * 100:.2f}%")
        print(f"Worst score:  {min(scores) * 100:.2f}%")
        print(f"Settings: [Mutation: {self.mutationRate * 100:.0f}% | Elite: {self.eliteSize}]")
        print("Best board:")
        print(Board(fitness_scores[0][0]))