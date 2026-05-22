import subprocess
import tempfile
import os
import re
import sys

def get_se_rating(sudoku_string: str) -> float:
    """
    Oblicza rating SE (Sudoku Explainer) dla podanego stringu Sudoku (81 znaków, 0 to puste pola).
    Wymaga zainstalowanego środowiska Java oraz pliku ratera (domyślnie sukakuexplainer.jar).
    
    :param sudoku_string: 81-znakowy ciąg reprezentujący planszę Sudoku (0 to puste pola).
    :param jar_path: Ścieżka do pliku sukakuexplainer.jar.
    :return: Zwraca wartość float reprezentującą rating SE (Explainer Rating).
    """
    jar_path = "src/Rating/SukakuExplainer.jar"
    if len(sudoku_string) != 81:
        raise ValueError("String Sudoku musi mieć dokładnie 81 znaków.")
    
    if not os.path.exists(jar_path):
        raise FileNotFoundError(f"Nie znaleziono pliku {jar_path}. Upewnij się, że rater znajduje się w podanej ścieżce.")
    
    # Utworzenie plików tymczasowych
    fd_in, temp_in = tempfile.mkstemp(suffix=".txt", text=True)
    fd_out, temp_out = tempfile.mkstemp(suffix=".txt", text=True)
    
    try:
        # Zapisz sudoku do pliku wejściowego
        with os.fdopen(fd_in, 'w') as f:
            f.write(sudoku_string + '\n')
            
        # Wywołaj proces Java
        # Polecenie: java -Xmx500m -cp sukakuexplainer.jar diuf.sudoku.test.serate --input=temp_in --output=temp_out
        command = [
            "java",
            "-Xmx500m",
            "-cp",
            jar_path,
            "diuf.sudoku.test.serate",
            f"--input={temp_in}",
            f"--output={temp_out}"
        ]
        
        # Wykonaj polecenie, ignorując standardowe wyjście, aby zachować czystość konsoli
        process = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        
        # Odczytaj wygenerowany plik wyjściowy
        if not os.path.exists(temp_out):
            raise RuntimeError("Plik wyjściowy nie został wygenerowany przez ratera.")
            
        with open(temp_out, 'r') as f:
            output_content = f.read().strip()
            
        if not output_content:
            error_msg = process.stderr if process.stderr else "Nieznany błąd podczas działania ratera."
            raise RuntimeError(f"Rater nie zwrócił żadnego wyniku. Możliwy błąd Javy:\n{error_msg}")
            
        # Format wyjściowy to: <ER>/<EP>/<ED>
        # Przykład: 4.2/1.2/1.2
        # Interesuje nas pierwsza liczba (ER - Explainer Rating)
        
        # Dzielimy string po ukośniku i bierzemy pierwszy element
        parts = output_content.split('/')
        
        if len(parts) >= 1:
            try:
                return float(parts[0].strip())
            except ValueError:
                raise RuntimeError(f"Nie udało się przekonwertować wartości '{parts[0]}' na liczbę. Pełny wynik: {output_content}")
                
        raise RuntimeError(f"Nieznany format wyniku: {output_content}")
            
    finally:
        # Usunięcie plików tymczasowych
        if os.path.exists(temp_in):
            os.remove(temp_in)
        if os.path.exists(temp_out):
            os.remove(temp_out)

if __name__ == "__main__":
    print("=" * 60)
    print("TESTOWANIE FUNKCJI Sudoku Explainer Rating")
    print("=" * 60)
    
    # Nazwa pliku ratera - zmień, jeśli plik ma inną nazwę.
    
        
        # Opcjonalnie: Tworzymy pusty plik "sukakuexplainer.jar" do celów demonstracyjnych
        # usuniemy go później, aby nie mylić użytkownika
        # Jednak dla bezpieczeństwa lepiej po prostu przerwać wykonanie
    
    # Przykładowe zestawy Sudoku do testowania (0 to puste pola)
    test_sudokus = [
        {
            "nazwa": "Łatwe (Easy)",
            "grid": "003020600900305001001806400008102900700000008006708200002609500800203009005010300"
        },
        {
            "nazwa": "Trudne (Hard) 4.2 - wmagające zaawansowanych technik",
            "grid": "060050030000306000007000400030000060014020790700000001000000000900147005051609870"
        },
        {
            "nazwa": "AI Escargot (Niezwykle trudne - SE >10.0)",
            "grid": "100007090030020008009600500005300900010080002600004000300000010040000007007000300"
        }
    ]
    
    for i, test in enumerate(test_sudokus, 1):
        print(f"\n--- Test {i}: {test['nazwa']} ---")
        print(f"Sudoku string : {test['grid']}")
        
        # Jeżeli jar istnieje, wywołujemy właściwą funkcję
    
        try:
            rating = get_se_rating(test['grid'])
            print(f"✅ Otrzymany rating SE: {rating}")
        except Exception as e:
            print(f"❌ Wystąpił błąd podczas oceny: {e}")
        
    print("\nZakończono testy.")