class Cell:
    def __init__(self, value: str):
        self.value: str = value
        self.candidates: list[str] = []

    def __str__(self) -> str:
        return self.value