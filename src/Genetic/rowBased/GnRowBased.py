import random
from src.Board import Board
from src.Genetic.Genetic import Genetic
from src.Genetic.rowBased.Crossover import crossoverType, getCrossoverMethod

class GnRowBased(Genetic[list[str]]):
  def __init__(self, populationSize: int, mutationRate: float, generations: int, eliteSize: int, crossoverFunctionName: crossoverType):
    self.nr = 9
    self.crossoverFunc = getCrossoverMethod(crossoverFunctionName)
    super().__init__(populationSize, mutationRate, generations, eliteSize, individualType=list[str])
   
  def createRandomIndividual(self) -> list[str]:
    board = []
    for _ in range(self.nr):
      row = [str(i) for i in range(1, self.nr + 1)]
      random.shuffle(row)
      board.extend(row)
    return board

  def crossover(self, parent1: list[str], parent2: list[str]) -> list[str]:
    return self.crossoverFunc(parent1, parent2)

  def fitness(self, population: list[list[str]]) -> list[tuple[list[str], float]]:
    fit = []
    for board in population:
      score = 0.0
      for col in range(9):
          column_vals = set(board[col::9]) 
          score += len(column_vals)        
      for box_row in range(3):
          for box_col in range(3):
              box_vals = set()
              for r in range(3):
                  idx = (box_row * 3 + r) * 9 + (box_col * 3)
                  box_vals.update(board[idx : idx + 3])
              score += len(box_vals)
      fit.append(score / 162.0)
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

  def mutate(self, individual: list[str]) -> list[str]:
    size = int(len(individual) ** (1/2)) #
    for i in range(size):
      if random.random() < self.mutationRate:
          idx1 = random.randint(i * size, (i + 1) * size - 1)
          idx2 = random.randint(i * size, (i + 1) * size - 1)
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
        pretty_board = Board(fitness[0][0])
        print("Best board:")
        print(pretty_board)