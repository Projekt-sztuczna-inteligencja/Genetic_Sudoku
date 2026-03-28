import random
from src.Board import Board
from src.Genetic.Crossover import crossoverType, getCrossoverMethod
from src.Genetic.Genetic import Genetic

class GeneticSudoku(Genetic[Board]):
  def __init__(self, populationSize: int, mutationRate: float, generations: int, eliteSize: int,
               crossoverFunctionName: crossoverType,):
    super().__init__(populationSize, mutationRate, generations, eliteSize, individualType=Board)
    self.crossoverFunc = getCrossoverMethod(crossoverFunctionName)

  def crossover(self, parent1: Board, parent2: Board) -> Board:
    return self.crossoverFunc(parent1, parent2)

  def fitness(self, population: list[Board]) -> list[tuple[Board, float]]:
    fit = []
    for board in population:
        size = board.size ** 2
        score = size
        for i in range(size):
            if not board.cell_valid(i):
                score -= 1
        score /= size
        fit.append(score)
    
    ret = list(zip(population, fit))
    ret.sort(key=lambda x: x[1], reverse=True)
    return ret

  def selectParents(self, population: list[tuple[Board, float]], nrParents: int) -> list[Board]:
    parents : list[Board] = []
    for _ in range(nrParents):
        tournament = random.sample(population, 4)
        winner = max(tournament, key=lambda x: x[1])[0]
        parents.append(winner)
    return parents

  def mutate(self, individual: Board) -> Board:
    for squareNumber in range(individual.size):
      if random.random() < self.mutationRate:
          square = individual.get_square(squareNumber)
          idx1 = random.randint(0, individual.size - 1)
          idx2 = random.randint(0, individual.size - 1)
          square[idx1].value, square[idx2].value = square[idx2].value, square[idx1].value
    return individual

  def printPopulation(self, nr = 0):
        
        fitenss = self.fitness(self.population)
        score = [individual[1] for individual in fitenss]

        print(f"---- Generation {nr} ----")
        print(f"Best score: {max(score) * 100:.2f}%")
        print(f"Worst score: {min(score) * 100:.2f}%")
        print(f"Median score: {sorted(score)[len(score) // 2] * 100:.2f}%")
        print(f"Average score: {sum(score) / len(score) * 100:.2f}%")
        print(f"Used Methods: {self.crossoverFunc.__name__} | Mutation Rate: {self.mutationRate * 100:.2f}% | Elite Size: {self.eliteSize}")
        print("Best board:")
        bestBoard = self.population[score.index(max(score))]
        print(bestBoard)
  
  def createRandomIndividual(self) -> Board:
    individual = Board()
    individual.random()
    return individual