import random
from abc import ABC, abstractmethod
from time import time
from typing import Generic, TypeVar

T = TypeVar('T')


class Genetic(ABC, Generic[T]):
    """Klasa bazowa definiująca główną pętlę i strukturę algorytmu genetycznego."""

    def __init__(self, populationSize: int, mutationRate: float, generations: int, eliteSize: int,
                 individualType: type[T]):
        self.populationSize = populationSize
        self.mutationRate = mutationRate
        self.generations = generations
        self.eliteSize = eliteSize
        self.T = individualType
        self.population: list[T] = []

        # Zmienne do monitorowania uwięzienia algorytmu (minima lokalne)
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
    def printPopulation(self, nr: int):
        pass

    @abstractmethod
    def createRandomIndividual(self) -> T:
        pass

    def run(self, stop_event=None):
        """Główna metoda uruchamiająca ewolucję."""
        self.createPopulation()
        start_time = time()

        for i in range(self.generations):
            # Przerwanie ewolucji, jeśli inny rdzeń odniósł sukces
            if stop_event is not None and stop_event.is_set():
                return None

            # Drukuj postęp co 100 pokoleń
            if i % 100 == 0:
                self.printPopulation(i)
                print(f"Time since last print: {time() - start_time:.2f} seconds")
                start_time = time()

            self.population = self.nextGeneration()

            # Wygrywamy, jeśli cała populacja to już tylko idealne rozwiązanie
            if len(self.population) == 1:
                return self.population[0]

    def createPopulation(self):
        """Inicjalizuje początkową populację."""
        for _ in range(self.populationSize):
            temp = self.createRandomIndividual()
            self.population.append(temp)

    def crossoverPopulation(self, parents: list[T], nrChildren: int) -> list[T]:
        """Krzyżuje rodziców w celu uzyskania nowego pokolenia."""
        if nrChildren == 0:
            return []

        parentPool = parents[:]
        random.shuffle(parentPool)

        children = []
        for childIdx in range(nrChildren):
            parent1 = parentPool[childIdx % len(parentPool)]
            parent2 = parentPool[-(childIdx % len(parentPool)) - 1]
            children.append(self.crossover(parent1, parent2))
        return children

    def mutatePopulation(self, individuals: list[T]):
        """Aplikuje prawdopodobieństwo mutacji na zadaną populację."""
        newPopulation = []
        for individual in individuals:
            newPopulation.append(self.mutate(individual))
        return newPopulation

    def nextGeneration(self):
        """Tworzy kolejne pokolenie z zachowaniem elity i ochrony przed stagnacją."""
        scoredPopulation = self.fitness(self.population)

        # Warunek 100% poprawności
        if scoredPopulation[0][1] == 1.0:
            return [scoredPopulation[0][0]]

        best_score = scoredPopulation[0][1]

        # Monitorowanie stagnacji populacji
        if best_score == self.last_best_score:
            self.stagnation_count += 1
        else:
            self.last_best_score = best_score
            self.stagnation_count = 0

        sortedPopulation = [i[0] for i in scoredPopulation]
        elite: list[T] = sortedPopulation[: self.eliteSize]

        # Wymieranie (Katastrofa) w razie braku postępu
        if self.stagnation_count > 30:
            self.stagnation_count = 0

            # Próba memetycznej naprawy lidera (jeśli zaimplementowana w podklasie)
            if hasattr(self, 'localSearch'):
                elite[0] = self.localSearch(elite[0])

            # Lider przetrwał, reszta wymiera (zastępujemy nowymi losowymi osobnikami)
            newPopulation = [elite[0]]
            for _ in range(self.populationSize - 1):
                newPopulation.append(self.createRandomIndividual())
            return newPopulation

        # Standardowe rozmnażanie i mutacje
        parents = self.selectParents(scoredPopulation, self.populationSize - self.eliteSize)
        children = self.crossoverPopulation(parents, self.populationSize - self.eliteSize)
        children = self.mutatePopulation(children)

        return elite + children