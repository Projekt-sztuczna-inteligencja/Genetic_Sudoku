import random
from src.Board import Board

class Genetic:
    def __init__(self, population_size, mutation_rate, generations, elite_size):
        self.population_size = population_size
        self.mutation_rate = mutation_rate
        self.generations = generations
        self.elite_size = elite_size
        self.population : list[Board] = []
        self.score = []

    def create_population(self):
        for _ in range(self.population_size):
            temp = Board()
            self.population.append(temp)
            self.population[-1].random()

    def fitness(self):
        self.score = []
        for board in self.population:
            size = board.size ** 2
            score = size
            for i in range(size):
                if not board.cell_valid(i):
                    score -= 1
            score /= size
            self.score.append(score)

    def select_parents(self, population, n_parents) -> list[Board]:
        parents = []
        for _ in range(n_parents):
            tournament = random.sample(population, 4)
            winner = max(tournament, key=lambda board: self.score[population.index(board)])
            parents.append(winner)
        return parents

    def crossoverPopulation(self, parents: list[Board], n_children: int) -> list[Board]:
        children = []
        if n_children == 0:
            return []

        parent_pool = [i for i in parents]
        random.shuffle(parent_pool)

        children = []
        for child_index in range(n_children):
            parent1 = parent_pool[child_index % len(parent_pool)]
            parent2 = parent_pool[-(child_index % len(parent_pool)) - 1]
            children.append(self.crossover(parent1, parent2))
        return children

    def crossover(self, parent1, parent2):
        child = Board()
        maxsquaresfromp1 = child.size - 2
        squaresP1 = random.randint(1, maxsquaresfromp1)
        p1 = random.sample(range(child.size), squaresP1)
        squares = [[] for _ in range(child.size)]
        for i in range(child.size):
            if i in p1:
                squares[i] = parent1.get_square(i)
            else:
                squares[i] = parent2.get_square(i)
        for sq in range(len(squares)):
            childSquare = child.get_square(sq)
            for nr in range(len(squares[sq])):
                childSquare[nr].value = squares[sq][nr].value
        return child

    # def crossover(self, parent1, parent2): # one point crossover
    #     child = Board()
    #     crossover_point = random.randint(1, child.size - 1)
    #     for i in range(child.size):
    #         if i < crossover_point:
    #             square = parent1.get_square(i)
    #         else:
    #             square = parent2.get_square(i)
    #         childSquare = child.get_square(i)
    #         for nr in range(len(square)):
    #             childSquare[nr].value = square[nr].value
    #     return child
    
    # def crossover(self, parent1, parent2): # two point crossover
    #     child = Board()
    #     crossover_point1 = random.randint(1, child.size - 2)
    #     crossover_point2 = random.randint(crossover_point1 + 1, child.size - 1)
    #     for i in range(child.size):
    #         if i < crossover_point1 or i >= crossover_point2:
    #             square = parent1.get_square(i)
    #         else:
    #             square = parent2.get_square(i)
    #         childSquare = child.get_square(i)
    #         for nr in range(len(square)):
    #             childSquare[nr].value = square[nr].value
    #     return child
    
    # def crossover(self, parent1, parent2): # row crossover
    #     child = Board()
    #     point = random.randint(1, (int)(child.size ** (1/2)))
    #     for i in range(child.size):
    #         if i < point * (child.size ** (1/2)):
    #             square = parent1.get_square(i)
    #         else:
    #             square = parent2.get_square(i)
    #         childSquare = child.get_square(i)
    #         for nr in range(len(square)):
    #             childSquare[nr].value = square[nr].value
    #     return child



    def mutate(self, board):
        import random
        for squareNumber in range(board.size):
            if random.random() < self.mutation_rate:
                square = board.get_square(squareNumber)
                idx1 = random.randint(0, board.size - 1)
                idx2 = random.randint(0, board.size - 1)
                square[idx1].value, square[idx2].value = square[idx2].value, square[idx1].value

    def rankPopulation(self) -> list[Board]:
        self.fitness()
        ranked = sorted(zip(self.population, self.score), key=lambda x: x[1], reverse=True)
        return [board for board, score in ranked]

    def nextGeneration(self):
        # elita <- tok_n_elite(populacja) (n_elite)
        # rodzice <- selekcja(populacja, n_parents) (np. turniejowa) n_parents = n_children = pop_size - n_elite
        # dzieci <- crossover_population(rodzice, n_children)
        # dzieci <- mutacja(dzieci) (z mut. rate)
        # new_population <- elita + dzieci
        new_population : list[Board] = []
        self.population = self.rankPopulation()
        elite : list[Board] = self.population[: self.elite_size]
        parents = self.select_parents(self.population, self.population_size - self.elite_size)
        children = self.crossoverPopulation(parents, self.population_size - self.elite_size)
        for child in children:
            self.mutate(child)
        new_population.extend(elite)
        new_population.extend(children)
        self.population = new_population

    def print_population(self, nr = 0):
        self.fitness()
        # for i in range(0, len(self.population), self.population_size % 10 or 10):
        #     print(f"[{i}] score: {self.score[i] * 100:.2f}%")
        print(f"---- Generation {nr} ----")
        print(f"Best score: {max(self.score) * 100:.2f}%")
        print(f"Worst score: {min(self.score) * 100:.2f}%")
        print(f"Median score: {sorted(self.score)[len(self.score) // 2] * 100:.2f}%")
        print(f"Average score: {sum(self.score) / len(self.score) * 100:.2f}%")
        print("Best board:")
        best_board = self.population[self.score.index(max(self.score))]
        print(best_board)

    def run(self):
        self.create_population()
        for i in range(self.generations):
            if i % 100 == 0:
                self.print_population(i)
            self.nextGeneration()