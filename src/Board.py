from src.Cell import Cell
from typing import Optional

class Board:
    def __init__(self, characters: Optional[list[str]] = None):
        if characters is None:
            characters = ['1', '2', '3', '4', '5', '6', '7', '8', '9']
            
        self.characters: list[str] = characters
        self.size: int = len(self.characters)
        
        self.cells: list[Cell] = [Cell(' ') for _ in range(self.size ** 2)]

    def write(self, x: int, y: int, value: str):
        if value not in self.characters:
            raise ValueError(f"Wartość {value} nie znajduje się w dozwolonych znakach!")
        self.get_cell(x, y).value = value

    def get_cell(self, x: int, y: int) -> Cell:
        return self.cells[y * self.size + x]

    def get_column(self, x: int) -> list[Cell]:
        return [self.get_cell(x, y) for y in range(self.size)]

    def get_row(self, y: int) -> list[Cell]:
        return [self.get_cell(x, y) for x in range(self.size)]

    def get_square(self, nr: int) -> list[Cell]:
        ret = []
        square_size = int(self.size ** 0.5)
        x0 = (nr % square_size) * square_size
        y = (nr // square_size) * square_size
        
        for _ in range(square_size):
            x = x0
            for _ in range(square_size):
                ret.append(self.get_cell(x, y))
                x += 1
            y += 1
        return ret
      
    def __str__(self) -> str:
        rows = []
        for y in range(self.size):
            row_chars = [
                self.get_cell(x, y).value if self.get_cell(x, y).value != ' ' else '.'
                for x in range(self.size)
            ]
            rows.append(" ".join(row_chars))
        return "\n".join(rows) + "\n"