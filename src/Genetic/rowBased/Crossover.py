import random
from typing import Callable, Literal


crossoverType = Literal[ "onepoint", "twopoint", "square"]

def onePoint(parent1: list[str], parent2: list[str]) -> list[str]:
    row_point = random.randint(1, 8)
    split_idx = row_point * 9
    child = parent1[:split_idx] + parent2[split_idx:]
    return child


def twoPoint(parent1: list[str], parent2: list[str]) -> list[str]:
    row1 = random.randint(1, 7)
    row2 = random.randint(row1 + 1, 8)
    idx1 = row1 * 9
    idx2 = row2 * 9
    child = parent1[:idx1] + parent2[idx1:idx2] + parent1[idx2:]
    return child
def square(parent1: list[str], parent2: list[str]) -> list[str]:
    band = random.randint(1, 2)
    crossover_point = band * 27 
    child = parent1[:crossover_point] + parent2[crossover_point:]
    return child

CROSSOVER_REGISTRY: dict[str, Callable[[list[str], list[str]], list[str]]] = {
    "onepoint": onePoint,
    "twopoint": twoPoint,
    "square": square,  
}

def getCrossoverMethod(name: crossoverType) -> Callable[[list[str], list[str]], list[str]]:
    if name not in CROSSOVER_REGISTRY:
        raise ValueError(f"Nieznana metoda: {name}")
    return CROSSOVER_REGISTRY[name]
