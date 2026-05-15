import ctypes
import os
import sys

# 1. Załadowanie odpowiedniej biblioteki w zależności od systemu
lib_path = os.path.abspath('src/Rating/libsudoku.dll')


# try:
sudoku_lib = ctypes.CDLL(lib_path)
# except OSError:
#     print(f"Nie znaleziono pliku biblioteki: {lib_path}. Upewnij się, że kod został skompilowany.")
#     sys.exit(1)

# 2. Skonfigurowanie sygnatury dla funkcji getSERating
# C: float getSERating(char* originalSudoku, SudokuMethodRecord records[], int methodsCount);
sudoku_lib.getSERating.argtypes = [ctypes.c_char_p, ctypes.c_void_p, ctypes.c_int]
sudoku_lib.getSERating.restype = ctypes.c_float

# 3. Pobranie globalnych zmiennych z C do użycia w module Rater
try:
    # Poprawka: Tworzymy 1-bajtową "wirtualną" tablicę tylko po to, 
    # aby bezpiecznie złapać adres w pamięci (zamiast czytać jej zawartość).
    method_array = (ctypes.c_byte * 1).in_dll(sudoku_lib, "methodRecords")
    
    # Przekształcamy fizyczny adres pamięci w typ c_void_p
    c_method_records = ctypes.c_void_p(ctypes.addressof(method_array))
    
    # Ilość metod (int)
    c_num_methods = ctypes.c_int.in_dll(sudoku_lib, "numMethods").value
except ValueError as e:
    print("Błąd ładowania zmiennych globalnych z pliku C.", e)
    sys.exit(1)

def get_sudoku_rating(board_string: str) -> float:
    """
    Oblicza trudność planszy Sudoku (SE Rating).
    Zwraca float z wynikiem. Jeśli tablica wymaga trudniejszych technik, zwraca -1.0.
    """
    if len(board_string) != 81:
        raise ValueError("Plansza Sudoku musi składać się dokładnie z 81 znaków.")
    
    # Poprawka: create_string_buffer tworzy bezpieczny ciąg znaków w stylu języka C
    # z gwarantowanym na końcu "znakiem pustym" (\0), chroniąc C przed błędem przepełnienia.
    b_board = ctypes.create_string_buffer(board_string.encode('utf-8'))
    
    rating = sudoku_lib.getSERating(b_board, c_method_records, c_num_methods)
    return rating

# =====================================================================
# Przykład użycia
# =====================================================================
if __name__ == "__main__":
    # Pusta plansza to 81 zer
    # Poniżej przykładowe, łatwe Sudoku:
    sample_sudoku = (
        "003020600"
        "900305001"
        "001806400"
        "008102900"
        "700000008"
        "006708200"
        "002609500"
        "800203009"
        "005010300"
    )

    print("Analiza planszy Sudoku...")
    rating = get_sudoku_rating(sample_sudoku)
    
    if rating > 0:
        print(f"Obliczony SE Rating: {rating}")
    else:
        print("Nie udało się rozwiązać Sudoku - plansza wymaga metod spoza zakresu lub jest nieprawidłowa.")