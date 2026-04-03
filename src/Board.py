class Board:
    def __init__(self, board_data: list[str]):
        # Przyjmujemy płaską listę 81 stringów wygenerowaną przez algorytm
        if len(board_data) != 81:
            raise ValueError("Plansza musi mieć dokładnie 81 elementów!")
        self.board_data = board_data

    def _is_valid(self, index: int) -> bool:
        """Prywatna metoda sprawdzająca, czy komórka nie ma duplikatów w wierszu, kolumnie i kwadracie."""
        value = self.board_data[index]
        
        # Traktujemy puste miejsca jako błędy
        if value in (' ', '.', '0'):
            return False 

        row = index // 9
        col = index % 9
        
        # 1. Sprawdzanie wiersza
        for c in range(9):
            if c != col and self.board_data[row * 9 + c] == value:
                return False
                
        # 2. Sprawdzanie kolumny
        for r in range(9):
            if r != row and self.board_data[r * 9 + col] == value:
                return False
                
        # 3. Sprawdzanie kwadratu 3x3
        start_row = (row // 3) * 3
        start_col = (col // 3) * 3
        for r in range(start_row, start_row + 3):
            for c in range(start_col, start_col + 3):
                idx = r * 9 + c
                if idx != index and self.board_data[idx] == value:
                    return False
                    
        return True

    def __str__(self) -> str:
        # Kody ANSI do kolorowania tekstu w konsoli
        RED = '\033[91m'
        GREEN = '\033[92m'
        RESET = '\033[0m'
        
        errors = 0
        lines = []
        
        for r in range(9):
            # Rysowanie poziomych linii oddzielających kwadraty 3x3
            if r % 3 == 0 and r != 0:
                lines.append("-" * 21)
            
            row_str = []
            for c in range(9):
                # Rysowanie pionowych linii oddzielających kwadraty 3x3
                if c % 3 == 0 and c != 0:
                    row_str.append("|")
                
                idx = r * 9 + c
                val = self.board_data[idx]
                
                # Kolorowanie i zliczanie błędów
                if self._is_valid(idx):
                    row_str.append(f"{GREEN}{val}{RESET}")
                else:
                    row_str.append(f"{RED}{val}{RESET}")
                    errors += 1
            
            lines.append(" ".join(row_str))
            
        # Dodanie podsumowania na dole
        lines.append(f"\n{RED}Liczba niepoprawnych komórek: {errors}{RESET}")
        if errors == 0:
            lines.append(f"{GREEN}SUDOKU ROZWIĄZANE IDEALNIE!{RESET}")
            
        return "\n".join(lines) + "\n"