import random
from abc import ABC, abstractmethod
from time import time
from typing import Generic, TypeVar

T = TypeVar('T')

class Genetic(ABC, Generic[T]):
    def __init__(self, populationSize: int, mutationRate: float, generations: int, eliteSize: int,
                 individualType: type[T]):
        self.populationSize = populationSize
        self.mutationRate = mutationRate
        self.generations = generations
        self.eliteSize = eliteSize
        self.T = individualType  # Zostawiamy to, żeby móc tworzyć nowe obiekty w createPopulation
        self.population: list[T] = []

    @abstractmethod
    def crossover(self, parent1: T, parent2: T) -> T:
        pass

    @abstractmethod
    def fitness(self, population: list[T]) -> list[tuple[T, float]]:
        pass

    @abstractmethod
    def selectParents(self, population: list[tuple[T, float]], nrParents: int) -> list[T]:
        pass

    @abstractmethod
    def mutate(self, individual: T) -> T:
        pass
    @abstractmethod
    def printPopulation(self, nr: int):
        pass

    @abstractmethod
    def createRandomIndividual(self) -> T:
        pass

    # run the algorithm
    # run the algorithm
    def run(self, stop_event=None):
        self.createPopulation()
        start_time = time()
        for i in range(self.generations):

            if stop_event is not None and stop_event.is_set():
                return None

            if i % 100 == 0:
                self.printPopulation(i)
                print(f"Time since last print: {time() - start_time:.2f} seconds\n")
                start_time = time()
            self.population = self.nextGeneration()
            if len(self.population) == 1:
                return self.population[0]

    # creates initial population
    def createPopulation(self):
        for _ in range(self.populationSize):
            temp = self.createRandomIndividual()
            self.population.append(temp)

    def crossoverPopulation(self, parents: list[T], nrChildren: int) -> list[T]:
        children = []
        if nrChildren == 0:
            return []

        parentPool = [i for i in parents]
        random.shuffle(parentPool)

        children = []
        for childIdx in range(nrChildren):
            parent1 = parentPool[childIdx % len(parentPool)]
            parent2 = parentPool[-(childIdx % len(parentPool)) - 1]
            children.append(self.crossover(parent1, parent2))
        return children

    def mutatePopulation(self, individuals: list[T]):
        newPopulation = []
        for individual in individuals:
            newPopulation.append(self.mutate(individual))
        return newPopulation

        # returns sorted population based on fitness score
    # def sortPopulation(self, population: list[T]) -> list[T]:
    #     ranked = self.fitness(population)
    #     return [individual[0] for individual in ranked]

    def nextGeneration(self):
        newPopulation: list[T] = []
        scoredPopulation = self.fitness(self.population)

        if scoredPopulation[0][1] == 1.0:
            return [scoredPopulation[0][0]]

        best_score = scoredPopulation[0][1]

        # --- MECHANIZM KATASTROFY + LOCAL SEARCH ---
        if not hasattr(self, 'stagnation_count'):
            self.stagnation_count = 0
            self.last_best_score = best_score

        if best_score == self.last_best_score:
            self.stagnation_count += 1
        else:
            self.last_best_score = best_score
            self.stagnation_count = 0

        sortedPopulation = [i[0] for i in scoredPopulation]
        elite: list[T] = sortedPopulation[: self.eliteSize]

        # Gdy utknęliśmy na 50 pokoleń - Memetic Local Search + Meteoryt!
        if self.stagnation_count > 30:
            self.stagnation_count = 0

            # Algorytm Memetyczny: odpalamy logiczne przeszukiwanie dla lidera
            if hasattr(self, 'localSearch'):
                elite[0] = self.localSearch(elite[0])

            newPopulation = [elite[0]]  # Zostawiamy TYLKO wybitnego lidera
            # Resztę populacji (1999 osobników) losujemy od nowa (świeża krew!)
            for _ in range(self.populationSize - 1):
                newPopulation.append(self.createRandomIndividual())
            return newPopulation
        # -------------------------------------------

        parents = self.selectParents(scoredPopulation, self.populationSize - self.eliteSize)
        children = self.crossoverPopulation(parents, self.populationSize - self.eliteSize)
        children = self.mutatePopulation(children)
        newPopulation = elite + children
        return newPopulation

