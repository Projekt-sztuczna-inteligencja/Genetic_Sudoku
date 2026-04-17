import random
from typing import Callable, Literal

# Definicja dostępnych typów krzyżowania
crossoverType = Literal["onepoint", "twopoint", "square", "uniform"]

def onePoint(parent1: list[int], parent2: list[int]) -> list[int]:
    """Krzyżowanie jednopunktowe. Przecina planszę w losowym wierszu."""
    row_point = random.randint(1, 8)
    split_idx = row_point * 9
    child = parent1[:split_idx] + parent2[split_idx:]
    return child

def twoPoint(parent1: list[int], parent2: list[int]) -> list[int]:
    """Krzyżowanie dwupunktowe. Dzieli planszę na 3 pasy (góra, środek, dół)."""
    row1 = random.randint(1, 7)
    row2 = random.randint(row1 + 1, 8)
    idx1 = row1 * 9
    idx2 = row2 * 9
    child = parent1[:idx1] + parent2[idx1:idx2] + parent1[idx2:]
    return child

def square(parent1: list[int], parent2: list[int]) -> list[int]:
    """Krzyżowanie blokowe. Skleja wycinki rodziców dzieląc planszę poziomo co 3 wiersze."""
    band = random.randint(1, 2)
    crossover_point = band * 27
    child = parent1[:crossover_point] + parent2[crossover_point:]
    return child

def uniform(parent1: list[int], parent2: list[int]) -> list[int]:
    """Krzyżowanie jednorodne. Wybiera geny (kwadraty) losowo od obu rodziców."""
    child = []
    for i in range(9):
        # Rzut monetą dla każdego z 9 kwadratów niezależnie
        if random.random() < 0.5:
            child.extend(parent1[i*9 : (i+1)*9])
        else:
            child.extend(parent2[i*9 : (i+1)*9])
    return child

# Rejestr łączący stringi z faktycznymi funkcjami
CROSSOVER_REGISTRY: dict[str, Callable[[list[int], list[int]], list[int]]] = {
    "onepoint": onePoint,
    "twopoint": twoPoint,
    "square": square,
    "uniform": uniform
}

def getCrossoverMethod(name: crossoverType) -> Callable[[list[int], list[int]], list[int]]:
    """Pobiera funkcję krzyżującą z rejestru na podstawie jej nazwy."""
    if name not in CROSSOVER_REGISTRY:
        raise ValueError(f"Nieznana metoda: {name}")
    return CROSSOVER_REGISTRY[name]