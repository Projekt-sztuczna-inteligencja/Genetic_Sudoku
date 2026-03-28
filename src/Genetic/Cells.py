def _cell_valid(board: list[str], index: int) -> bool:
        """Sprawdza, czy cyfra pod danym indeksem nie powtarza się w wierszu, kolumnie i kwadracie 3x3"""
        value = board[index]
        
        # Jeśli pole jest puste (np. oznaczone jako '.'), od razu traktujemy je jako błędne
        if value == '.' or value == '0':
            return False 

        row = index // 9
        col = index % 9
        
        # 1. Sprawdzanie wiersza
        for c in range(9):
            if c != col and board[row * 9 + c] == value:
                return False
                
        # 2. Sprawdzanie kolumny
        for r in range(9):
            if r != row and board[r * 9 + col] == value:
                return False
                
        # 3. Sprawdzanie małego kwadratu 3x3
        start_row = (row // 3) * 3
        start_col = (col // 3) * 3
        for r in range(start_row, start_row + 3):
            for c in range(start_col, start_col + 3):
                idx = r * 9 + c
                if idx != index and board[idx] == value:
                    return False
                    
        return True