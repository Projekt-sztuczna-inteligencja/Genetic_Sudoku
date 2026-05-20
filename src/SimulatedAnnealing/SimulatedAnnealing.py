import math
import random
from abc import abstractmethod
from typing import TypeVar, Generic

T = TypeVar('T')

class SimulatedAnnealing(Generic[T]):
    def __init__(self, initialTemperature: float, coolingRate: float, iterations: int, type: type[T]):
        self.temperature = initialTemperature
        self.coolingRate = coolingRate
        self.iterations = iterations # Poprawiona literówka
        self.type = type
        self.individual = self.createRandomIndividual()
        self.score = self.cost(self.individual)
        
        # Pamiętamy najlepszy wynik, żeby go nie zgubić po drodze
        self.best_individual = self.individual[:] if isinstance(self.individual, list) else self.individual
        self.best_score = self.score

    def calculate_temperature(self) -> float:
        collection = [self.cost(self.createRandomIndividual()) for _ in range(100)]
        avg = sum(collection) / len(collection)
        collection = [(x - avg) ** 2 for x in collection]
        return (sum(collection)/ len(collection)) ** 0.5

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
        # USUNIĘTO: self.score > 0, ponieważ nasz koszty będą ujemne (np. -5.2)
        while i < self.iterations:
            newIndividual = self.change(self.individual)
            newScore = self.cost(newIndividual)
            
            if self.acceptance_probability(self.score, newScore) > random.random():
                self.individual = newIndividual
                self.score = newScore
                
                # Zapisujemy absolutny rekord
                if self.score < self.best_score:
                    self.best_score = self.score
                    self.best_individual = self.individual[:] if isinstance(self.individual, list) else self.individual

            self.temperature *= self.coolingRate
            i += 1
            
        return self.best_individual
    
    def acceptance_probability(self, currentScore: float, newScore: float) -> float:
        # Algorytm MINIMALIZUJE koszt. Jeśli nowy jest Mniejszy (np. -4.0 < -3.0), to bierzemy na 100%
        if newScore < currentScore:
            return 1.0
        # W przeciwnym razie akceptujemy gorszy krok z pewnym prawdopodobieństwem
        return math.exp((currentScore - newScore) / self.temperature)