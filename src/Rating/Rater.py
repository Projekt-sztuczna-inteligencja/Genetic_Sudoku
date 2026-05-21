import ctypes
import os
import sys
import time

# 1. Załadowanie odpowiedniej biblioteki w zależności od systemu
if sys.platform.startswith('win'):
    lib_path = os.path.abspath('src/Rating/libsudoku.dll')
else:
    lib_path = os.path.abspath('src/Rating/libsudoku.so')
try:
    sudoku_lib = ctypes.CDLL(lib_path)
except OSError:
    print(f"Nie znaleziono pliku biblioteki: {lib_path}. Upewnij się, że kod został skompilowany.")
    sys.exit(1)

# 2. Skonfigurowanie sygnatury dla funkcji getSERating
# C: float getSERating(char* originalSudoku, SudokuMethodRecord records[], int methodsCount);
sudoku_lib.getSERating.argtypes = [ctypes.c_char_p, ctypes.c_void_p, ctypes.c_int]
sudoku_lib.getSERating.restype = ctypes.c_float

# 3. Pobranie globalnych zmiennych z C do użycia w module Rater
try:
    method_array = (ctypes.c_byte * 1).in_dll(sudoku_lib, "methodRecords")
    c_method_records = ctypes.c_void_p(ctypes.addressof(method_array))
    c_num_methods = ctypes.c_int.in_dll(sudoku_lib, "numMethods").value
except ValueError as e:
    print("Błąd ładowania zmiennych globalnych z pliku C.", e)
    sys.exit(1)

def Rate_sudoku(board_string: str) -> float:
    """
    Oblicza trudność planszy Sudoku (SE Rating).
    Zwraca float z wynikiem. Jeśli tablica wymaga trudniejszych technik, zwraca -1.0.
    """
    if len(board_string) != 81:
        raise ValueError("Plansza Sudoku musi składać się dokładnie z 81 znaków.")
    
    b_board = ctypes.create_string_buffer(board_string.encode('utf-8'))
    rating = sudoku_lib.getSERating(b_board, c_method_records, c_num_methods)
    return rating

# =====================================================================
# Przykład użycia / Główny program diagnostyczny
# =====================================================================
if __name__ == "__main__":
    files_to_check = ['./Sudokus/easy.txt', './Sudokus/medium.txt', './Sudokus/hard.txt']
    
    print("=================================================")
    print("        SUDOKU SE GRADER - DIAGNOSTICS          ")
    print("=================================================\n")
    
    start_time = time.time()
    
    for filepath in files_to_check:
        print(f"[ANALYZING] {filepath}")
        print("-" * 49)
        
        if not os.path.exists(filepath):
            print(f"Brak pliku: {filepath}")
            print("-" * 49 + "\n")
            continue
            
        processed = 0
        solved = 0
        perfect_match = 0
        sum_deviation = 0.0
        max_deviation = 0.0
        
        with open(filepath, 'r', encoding='utf-8') as f:
            for line in f:
                line = line.strip()
                if not line:
                    continue
                
                parts = line.split()
                if len(parts) < 3:
                    continue
                    
                board_string = parts[1]
                try:
                    expected_rating = float(parts[2])
                except ValueError:
                    continue
                    
                processed += 1
                
                try:
                    computed_rating = Rate_sudoku(board_string)
                except ValueError:
                    continue
                
                if computed_rating != -1.0:
                    solved += 1
                    diff = abs(computed_rating - expected_rating)
                    
                    if diff < 0.01:
                        perfect_match += 1
                        
                    sum_deviation += diff
                    if diff > max_deviation:
                        max_deviation = diff
        
        solved_pct = (solved / processed * 100) if processed > 0 else 0.0
        perfect_pct = (perfect_match / solved * 100) if solved > 0 else 0.0
        avg_dev = (sum_deviation / solved) if solved > 0 else 0.0
        
        print(f"\n  Summary for {filepath}:")
        print(f"  > Puzzles processed:  {processed}")
        print(f"  > Solved by engine:   {solved} ({solved_pct:.1f}%)")
        print(f"  > Perfect SE Match:   {perfect_match} ({perfect_pct:.1f}%)")
        print(f"  > Avg SE Deviation:   {avg_dev:.3f}")
        print(f"  > Max SE Deviation:   {max_deviation:.1f}")
        print("-" * 49 + "\n")
        
    total_time = time.time() - start_time
    
    print(f"time taken: {total_time:.2f} seconds\n")
    print("All datasets processed.")