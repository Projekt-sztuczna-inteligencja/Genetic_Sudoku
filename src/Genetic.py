import src.Board as Board
class Genethic:
  def __init__(self, population_size, mutation_rate, generations):
      self.population_size = population_size
      self.mutation_rate = mutation_rate
      self.generations = generations
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
  def select_parents(self):
      total_score = sum(self.score)
      if total_score == 0:
          return self.population[0], self.population[0]
      probabilities = [score / total_score for score in self.score]
      import random
      parent1 = random.choices(self.population, probabilities)[0]
      parent2 = random.choices(self.population, probabilities)[0]
      return parent1, parent2
  def crossover(self, parent1, parent2):
      child = Board.Board()
      for i in range(len(parent1.cells)):
          child.cells[i].value = parent1.cells[i].value if i % 2 == 0 else parent2.cells[i].value
      return child
  def mutate(self, board):
      import random
      for cell in board.cells:
          if random.random() < self.mutation_rate:
              cell.value = random.choice(board.characters)
  def evolve(self):
      new_population = []
      for _ in range(self.population_size):
          parent1, parent2 = self.select_parents()
          child = self.crossover(parent1, parent2)
          self.mutate(child)
          new_population.append(child)
      self.population = new_population
      self.score = []
  def print_population(self):
      for i in range(len(self.population)):
          print(f"[{i}] score: {self.score[i] * 100:.2f}%")
      print(f"Best score: {max(self.score) * 100:.2f}%")
      print(f"Worst score: {min(self.score) * 100:.2f}%")
      print(f"Average score: {sum(self.score) / len(self.score) * 100:.2f}%")
  def run(self):
      self.create_population()
      for i in range(self.generations):
          if(i % (self.generations // 10) == 1):
              self.print_population()
          self.evolve()
          self.fitness()