import src.Board as Board
class Genethic:
  def __init__(self, population_size, mutation_rate):
      self.population_size = population_size
      self.mutation_rate = mutation_rate
      self.population = []
      self.score = []

  def create_population(self):
      for _ in range(self.population_size):
         self.population.append(Board.Board())
         self.population[-1].random()
    
  def fitness(self):
      for board in self.population:
          size = board.size ** 2
          score = size
          for i in range(size):
              
              if not board.cell_valid(i):
                  score -= 1
          score /= size
          self.score.append(score)
  def print_population(self):
      self.fitness()
      for i in range(len(self.population)):
          print(self.population[i])
          print(" score: " + str(self.score[i]))