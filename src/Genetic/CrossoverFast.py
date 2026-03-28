import random
from typing import Callable, Literal


crossoverType = Literal[ "onepoint", "twopoint", "square"]
# square-based crossover
# randomly selecting squares from each parent to form the child board.


def onePoint(parent1: list[str], parent2: list[str]) -> list[str]:
    # Losujemy wiersz przecięcia (od 1 do 8)
    row_point = random.randint(1, 8)
    
    # Przeliczamy wiersz na indeks w płaskiej liście (mnożymy razy 9)
    split_idx = row_point * 9
    
    # Sklejamy: początek od rodzica 1, reszta od rodzica 2
    child = parent1[:split_idx] + parent2[split_idx:]
    
    return child


def twoPoint(parent1: list[str], parent2: list[str]) -> list[str]:
    # Losujemy dwa wiersze przecięcia
    row1 = random.randint(1, 7)
    row2 = random.randint(row1 + 1, 8)
    
    # Przeliczamy na indeksy
    idx1 = row1 * 9
    idx2 = row2 * 9
    
    # Sklejamy: Rodzic 1 (góra) -> Rodzic 2 (środek) -> Rodzic 1 (dół)
    child = parent1[:idx1] + parent2[idx1:idx2] + parent1[idx2:]
    
    return child
def square(parent1: list[str], parent2: list[str]) -> list[str]:
    band = random.randint(1, 2)
    crossover_point = band * 27 
    
    # Tworzymy dziecko sklejając wycinek pierwszego rodzica z wycinkiem drugiego
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
