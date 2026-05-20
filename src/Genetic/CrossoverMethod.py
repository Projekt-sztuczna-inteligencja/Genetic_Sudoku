import random
from typing import Callable, Literal

crossoverType = Literal["onepoint", "twopoint", "square", "uniform"]

def onePoint(parent1: list[int], parent2: list[int]) -> list[int]:
    """Przecina planszę wzdłuż jednego losowego wiersza i łączy rodziców."""
    row_point = random.randint(1, 8)
    split_idx = row_point * 9
    return parent1[:split_idx] + parent2[split_idx:]

def twoPoint(parent1: list[int], parent2: list[int]) -> list[int]:
    """Dzieli planszę na trzy wierszowe paski, wymieniając środek pomiędzy rodzicami."""
    row1 = random.randint(1, 7)
    row2 = random.randint(row1 + 1, 8)
    idx1 = row1 * 9
    idx2 = row2 * 9
    return parent1[:idx1] + parent2[idx1:idx2] + parent1[idx2:]

def square(parent1: list[int], parent2: list[int]) -> list[int]:
    """Tworzy dziecko sklejając wycinek pierwszego rodzica z wycinkiem drugiego wzdłuż band (bloków 3x3)."""
    band = random.randint(1, 2)
    crossover_point = band * 27
    return parent1[:crossover_point] + parent2[crossover_point:]

def uniform(parent1: list[int], parent2: list[int]) -> list[int]:
    """Rzut monetą dla każdego z 9 kwadratów niezależnie, wybierając geny od pierwszego lub drugiego rodzica."""
    child = []
    for i in range(9):
        if random.random() < 0.5:
            child.extend(parent1[i * 9: (i + 1) * 9])
        else:
            child.extend(parent2[i * 9: (i + 1) * 9])
    return child

CROSSOVER_REGISTRY: dict[str, Callable[[list[int], list[int]], list[int]]] = {
    "onepoint": onePoint,
    "twopoint": twoPoint,
    "square": square,
    "uniform": uniform
}

def getCrossoverMethod(name: crossoverType) -> Callable[[list[int], list[int]], list[int]]:
    """Zwraca funkcję krzyżowania na podstawie podanej nazwy z rejestru."""
    if name not in CROSSOVER_REGISTRY:
        raise ValueError(f"Nieznana metoda krzyżowania: {name}")
    return CROSSOVER_REGISTRY[name]