import random
from src.Board import Board
from typing import Callable, Literal

crossoverType = Literal["square", "onepoint", "twopoint", "rowCrossover"]

# square-based crossover
# randomly selecting squares from each parent to form the child board.
def square(parent1, parent2):
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

# Performs one-point crossover 
# by choosing a crossover point 
# and combining squares from both parents up to that point.
def onePoint(parent1, parent2):
    child = Board()
    crossover_point = random.randint(1, child.size - 1)
    for i in range(child.size):
        if i < crossover_point:
            square = parent1.get_square(i)
        else:
            square = parent2.get_square(i)
        childSquare = child.get_square(i)
        for nr in range(len(square)):
            childSquare[nr].value = square[nr].value
    return child


# two-point crossover:
# selecting two crossover points 
# and combining squares from both parents between those points.
def twoPoint(parent1, parent2): 
    child = Board()
    crossover_point1 = random.randint(1, child.size - 2)
    crossover_point2 = random.randint(crossover_point1 + 1, child.size - 1)
    for i in range(child.size):
        if i < crossover_point1 or i >= crossover_point2:
            square = parent1.get_square(i)
        else:
            square = parent2.get_square(i)
        childSquare = child.get_square(i)
        for nr in range(len(square)):
            childSquare[nr].value = square[nr].value
    return child
    
# row-based crossover
# selecting a row crossover point 
# and combining rows from both parents to form the child.
def rowCrossover(parent1, parent2):
    child = Board()
    point = random.randint(1, (int)(child.size ** (1/2)))
    for i in range(child.size):
        if i < point * (child.size ** (1/2)):
            square = parent1.get_square(i)
        else:
            square = parent2.get_square(i)
        childSquare = child.get_square(i)
        for nr in range(len(square)):
            childSquare[nr].value = square[nr].value
    return child


CROSSOVER_REGISTRY: dict[str, Callable[[Board, Board], Board]] = {
    "square" : square,
    "onepoint": onePoint,
    "twopoint": twoPoint,
    "rowCrossover": rowCrossover
}

def getCrossoverMethod(name: crossoverType) -> Callable[[Board, Board], Board]:
    if name not in CROSSOVER_REGISTRY:
        raise ValueError(f"Nieznana metoda: {name}")
    return CROSSOVER_REGISTRY[name]
