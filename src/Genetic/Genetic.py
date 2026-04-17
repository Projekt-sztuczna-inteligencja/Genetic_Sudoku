import random
from abc import ABC, abstractmethod
from time import time
from typing import Generic, TypeVar

T = TypeVar('T')


class Genetic(ABC, Generic[T]):
    """Abstrakcyjna klasa bazowa zarządzająca pętlą algorytmu genetycznego."""

    def __init__(self, populationSize: int, mutationRate: float, generations: int, eliteSize: int,
                 individualType: type[T]):
        self.populationSize = populationSize
        self.mutationRate = mutationRate
        self.generations = generations
        self.eliteSize = eliteSize
        self.T = individualType
        self.population: list[T] = []
        self.stagnation_count = 0
        self.last_best_score = 0.0

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
    def createRandomIndividual(self) -> T:
        pass

    def run(self, stop_event=None):
        """Główna pętla wyścigu ewolucyjnego."""
        self.createPopulation()
        start_time = time()

        for i in range(self.generations):
            # Przerwanie działania, jeśli inna wyspa zgłosiła sukces
            if stop_event is not None and stop_event.is_set():
                return None

            # Wyświetlanie logów co 100 pokoleń
            if i % 100 == 0:
                if hasattr(self, 'printPopulation'):
                    self.printPopulation(i)
                print(f"Time since last print: {time() - start_time:.2f} seconds\n")
                start_time = time()

            self.population = self.nextGeneration()

            # Warunek sukcesu - cała populacja skurczyła się do jednego wygranego rozwiązania
            if len(self.population) == 1:
                return self.population[0]

    def createPopulation(self):
        """Generuje populację początkową."""
        self.population = [self.createRandomIndividual() for _ in range(self.populationSize)]

    def crossoverPopulation(self, parents: list[T], nrChildren: int) -> list[T]:
        """Tworzy dzieci łącząc wylosowane pary rodziców."""
        if nrChildren == 0:
            return []

        parentPool = parents[:]
        random.shuffle(parentPool)

        children = []
        pool_len = len(parentPool)
        for childIdx in range(nrChildren):
            parent1 = parentPool[childIdx % pool_len]
            parent2 = parentPool[-(childIdx % pool_len) - 1]
            children.append(self.crossover(parent1, parent2))
        return children

    def mutatePopulation(self, individuals: list[T]) -> list[T]:
        """Nakłada proces mutacji na całą listę dzieci."""
        return [self.mutate(ind) for ind in individuals]

    def nextGeneration(self) -> list[T]:
        """Tworzy kolejne pokolenie poprzez selekcję, krzyżowanie i mutacje (oraz heurystyki)."""
        scoredPopulation = self.fitness(self.population)
        best_score = scoredPopulation[0][1]

        # 1. Warunek zwycięstwa (100%)
        if best_score == 1.0:
            return [scoredPopulation[0][0]]

        # 2. Wykrywanie Stagnacji (Minima Lokalne)
        if best_score == self.last_best_score:
            self.stagnation_count += 1
        else:
            self.last_best_score = best_score
            self.stagnation_count = 0

        sortedPopulation = [i[0] for i in scoredPopulation]
        elite = sortedPopulation[:self.eliteSize]

        # 3. Mechanizm Katastrofy + Local Search (Algorytm Memetyczny)
        if self.stagnation_count > 50:
            self.stagnation_count = 0

            # Odpalamy logiczną naprawę błędów u lidera
            if hasattr(self, 'localSearch'):
                elite[0] = self.localSearch(elite[0])

            # Wymieranie: Lider przeżywa, reszta populacji to nowa krew
            newPopulation = [elite[0]]
            for _ in range(self.populationSize - 1):
                newPopulation.append(self.createRandomIndividual())
            return newPopulation

        # 4. Standardowa pętla reprodukcyjna
        parents = self.selectParents(scoredPopulation, self.populationSize - self.eliteSize)
        children = self.crossoverPopulation(parents, self.populationSize - self.eliteSize)
        children = self.mutatePopulation(children)

        return elite + children