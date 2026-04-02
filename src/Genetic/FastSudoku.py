import random
import ctypes
import os
from src.Board import Board
from src.Genetic.Genetic import Genetic
from src.Genetic.CrossoverFast import crossoverType, getCrossoverMethod

# 1. Absolutna ścieżka do folderu, w którym leży FastSudoku.py
current_dir = os.path.dirname(os.path.abspath(__file__))
dll_path = os.path.join(current_dir, "fitness_fast.dll")

# 2. Ładujemy bibliotekę z winmode=0 (wymagane w nowym Pythonie na Windowsie)
fast_c_lib = ctypes.CDLL(dll_path, winmode=0)


fast_c_lib.calculate_fitness.argtypes = [
    ctypes.POINTER(ctypes.c_int),
    ctypes.POINTER(ctypes.c_float),
    ctypes.c_int
]
fast_c_lib.calculate_fitness.restype = None

class FastSudoku(Genetic[list[int]]):
  def __init__(self, populationSize: int, mutationRate: float, generations: int, eliteSize: int, crossoverFunctionName: crossoverType,initial_board: list[int] = None):
    self.nr = 9
    self.crossoverFunc = getCrossoverMethod(crossoverFunctionName)
    self.initial_board = initial_board if initial_board is not None else [0]*81

    self.locked_position = [val != 0 for val in self.initial_board]
    super().__init__(populationSize, mutationRate, generations, eliteSize, individualType=list[int])

  def createRandomIndividual(self) -> list[str]:
    board = []
    for row_idx  in range(self.nr):
        row_start = row_idx*9
        original_row = self.initial_board[row_start:row_start+9]
        #Sprawdzanie jakich cyfr brakuje w wierszu
        missing_digits = [i for i in range(1,10) if i not in original_row]
        random.shuffle(missing_digits)

        new_row=[]
        missing_ptr=0
        for val in original_row:
            if val !=0:
                new_row.append(val)
            else:
                new_row.append(missing_digits[missing_ptr])
                missing_ptr+=1
        board.extend(new_row)
    return board

  def crossover(self, parent1: list[str], parent2: list[str]) -> list[str]:
    return self.crossoverFunc(parent1, parent2)
    # Losujemy punkt cięcia: po 1. paśmie (indeks 27) lub po 2. paśmie (indeks 54)



  def fitness(self, population: list[list[int]]) -> list[tuple[list[int], float]]:
    num_boards = len(population)

    #Spłaszczenie całej populacji do jednej listy
    flat_population = [val for board in population for val in board]

    #Konwersja listy na tablice C
    C_POP_ARRAY = ctypes.c_int * len(flat_population)
    C_SCORE_ARRAY = ctypes.c_float * num_boards

    c_pop = C_POP_ARRAY()
    c_pop[:] = flat_population

    c_scores = C_SCORE_ARRAY()

    #Wywołanie kodu C
    fast_c_lib.calculate_fitness(c_pop, c_scores, num_boards)

    #Złożenie wyników
    fit = [float(score) for score in c_scores]
    ret = list(zip(population, fit))
    ret.sort(key=lambda x: x[1], reverse=True)

    return ret


  def selectParents(self, population: list[tuple[list[str], float]], nrParents: int) -> list[list[str]]:
    parents : list[list[str]] = []
    for _ in range(nrParents):
        tournament = random.sample(population, 4)
        winner = max(tournament, key=lambda x: x[1])[0]
        parents.append(winner)
    return parents

    # --- NOWA FUNKCJA DIAGNOZUJĄCA ---
    # --- NOWA FUNKCJA DIAGNOZUJĄCA ---

  def get_detailed_errors(self, individual: list[int]):
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

    # ---------------------------------

    # --- TWOJA NOWA, INTELIGENTNA MUTACJA ---
  def mutate(self, individual: list[int]) -> list[int]:
      # Pobieramy konkretne indeksy, gdzie są błędy kolumn/wierszy
      error_idxs = self.get_detailed_errors(individual)

      # STRATEGIA A: Jeśli jest bardzo mało błędów (np. 4), naprawiamy je siłowo
      if 0 < len(error_idxs) <= 8:
          # Wybieramy jeden losowy błędny indeks
          target_idx = random.choice(error_idxs)
          sq_id = target_idx // 9  # ID kwadratu, w którym jest błąd

          # Szukamy w tym samym kwadracie innego nie-zablokowanego pola
          sq_start = sq_id * 9
          unlocked_in_sq = [
              i for i in range(sq_start, sq_start + 9)
              if not self.locked_position[i] and i != target_idx
          ]

          if unlocked_in_sq:
              swap_with = random.choice(unlocked_in_sq)
              individual[target_idx], individual[swap_with] = individual[swap_with], individual[target_idx]
              return individual

      # STRATEGIA B: Normalna mutacja snajperska dla reszty przypadków
      if random.random() < self.mutationRate:
          row_idx = random.randint(0, 8)
          row_start = row_idx * 9
          unlocked = [i for i in range(row_start, row_start + 9) if not self.locked_position[i]]
          if len(unlocked) >= 2:
              idx1, idx2 = random.sample(unlocked, 2)
              individual[idx1], individual[idx2] = individual[idx2], individual[idx1]

      return individual

  def printPopulation(self, nr = 0):

        fitness = self.fitness(self.population)
        score = [individual[1] for individual in fitness]

        print(f"---- Generation {nr} ----")
        print(f"Best score: {max(score) * 100:.2f}%")
        print(f"Worst score: {min(score) * 100:.2f}%")
        print(f"Median score: {sorted(score)[len(score) // 2] * 100:.2f}%")
        print(f"Average score: {sum(score) / len(score) * 100:.2f}%")
        print(f"Used Methods: row | Mutation Rate: {self.mutationRate * 100:.2f}% | Elite Size: {self.eliteSize}")
        print("Best board:")
        pretty_board = Board(fitness[0][0])
        print("Best board:")
        print(pretty_board)

  def localSearch(self, individual: list[int]) -> list[int]:
      current_best_ind = individual[:]
      # Pobieramy obecny wynik z C dla tego osobnika
      res = self.fitness([current_best_ind])
      current_best_score = res[0][1]

      # Próbujemy ulepszyć każdy kwadrat po kolei
      for sq_idx in range(9):
          sq_start = sq_idx * 9
          unlocked = [i for i in range(sq_start, sq_start + 9) if not self.locked_position[i]]

          if len(unlocked) < 2: continue

          # Generujemy WSZYSTKIE możliwe zamiany w tym kwadracie
          potential_neighbors = []
          for i in range(len(unlocked)):
              for j in range(i + 1, len(unlocked)):
                  neighbor = current_best_ind[:]
                  idx1, idx2 = unlocked[i], unlocked[j]
                  neighbor[idx1], neighbor[idx2] = neighbor[idx2], neighbor[idx1]
                  potential_neighbors.append(neighbor)

          if not potential_neighbors: continue

          # Wysyłamy całą paczkę do C do oceny
          scored_neighbors = self.fitness(potential_neighbors)

          # Jeśli najlepszy sąsiad jest lepszy niż to co mamy - bierzemy go!
          if scored_neighbors[0][1] > current_best_score:
              current_best_ind = scored_neighbors[0][0][:]
              current_best_score = scored_neighbors[0][1]

      return current_best_ind