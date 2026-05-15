import math
import random
from abc import abstractmethod
from typing import TypeVar

T = TypeVar('T')
class SimulatedAnnealing:
    def __init__(self, initialTemperature: int, coolingRate: int, iteratoins: int, type: type[T]):
        self.temperature = initialTemperature
        self.coolingRate = coolingRate
        self.iteratoins = iteratoins
        self.type = type
        self.individual = self.createRandomIndividual()
        self.score = self.cost(self.individual)

    def calcualte_temperature(self) -> float:
        collection = [self.cost(self.createRandomIndividual()) for _ in range(100)]
        avg = sum(collection) / len(collection)
        collection = [(x - avg) ** 2 for x in collection]
        return (sum(collection)/ len(collection)) ** (1/2)

    @abstractmethod
    def createRandomIndividual(self) -> T:
        pass
    
    @abstractmethod
    def cost(self, individual: T) -> float:
        pass
    
    @abstractmethod
    def change(self, individual: T) -> T:
        pass

    def run(self):
        i = 0
        while self.score > 0 and i < self.iteratoins:
            newIndividual = self.change(self.individual)
            newScore = self.cost(newIndividual)
            if self.acceptance_probability(self.score, newScore) > random.random():
                self.individual = newIndividual
                self.score = newScore
            self.temperature *= self.coolingRate
            i += 1
        return self.individual
    
    def acceptance_probability(self, currentScore: float, newScore: float) -> float:
        if newScore > currentScore:
            return 1.0
        return math.exp((newScore - currentScore) / self.temperature)