from concurrent.futures import ProcessPoolExecutor, as_completed
import time
# Importujemy Twoją klasę z pliku SASudoku.py
from src.SimulatedAnnealing.SASudoku import SA_SudokuGenerator
from src.Rating.Rater import Rate_sudoku

params = {
    'easy': {'temp':60.0, 'cooling': 0.84, 'iters': 100000, 'target': 1.5},
    'medium': {'temp': 70.0, 'cooling': 0.90, 'iters': 100000, 'target': 2.5},
    'hard': {'temp': 90.0, 'cooling': 0.97, 'iters': 100000, 'target': 3.5},
    'expert': {'temp': 100.0, 'cooling': 0.9995, 'iters': 100000, 'target': 5.0}
}

def generuj(params=params['easy']) -> dict:
    """
    Konfiguruje parametry algorytmu wyżarzania i generuje Sudoku.
    """
    solved_board = (
        "534678912"
        "672195348"
        "198342567"
        "859761423"
        "426853791"
        "713924856"
        "961537284"
        "287419635"
        "345286179"
    )

    if isinstance(params, str):
        if params in ['easy', 'medium', 'hard', 'expert']:
            params = params[params] 
        else:
            raise ValueError(f"Nieznany poziom trudności: {params}")

    generator = SA_SudokuGenerator(
        solved_board=solved_board,
        initialTemperature=params['temp'],
        coolingRate=params['cooling'],
        iterations=params['iters'],
        target_rating=params['target'],
        use_perfect_rater=True  # Używamy PerfectRater dla lepszej jakości oceny
    )
   
    best_mask = generator.run() 
    
    final_puzzle_str = generator.apply_mask_to_str(best_mask)
    final_rating = Rate_sudoku(final_puzzle_str)
    
    return {
        "target_rating": params['target'],
        "error": abs(final_rating - params['target']),
        "final_puzzle": final_puzzle_str,
        "final_rating": final_rating,
        "clues_count": sum(best_mask)
    }

# =====================================================================
# FUNKCJA WYKONYWANA W OSOBNYCH PROCESACH
# =====================================================================
def sprawdz_pojedynczy_kontekst(temp: float, cooling: float, iters: int, target_rating: float) -> dict:
    """
    Ta funkcja uruchamia się w kompletnie osobnym procesie (na osobnym rdzeniu).
    Brak problemu GIL!
    """
    test_params = {
        'temp': temp,
        'cooling': cooling,
        'iters': iters,
        'target': target_rating
    }
    
    t_start = time.time()
    res = generuj(test_params)
    t_duration = time.time() - t_start
    
    # Zwracamy surowe dane z powrotem do głównego procesu
    return {
        'temp': temp,
        'cooling': cooling,
        'iters': iters,
        'target_rating': target_rating,
        'final_rating': res['final_rating'],
        'error': res['error'],
        'clues_count': res['clues_count'],
        'duration': t_duration
    }


def optymalizuj_parametry() -> dict:
    # 1. Przygotowanie zakresów pętli
    temp_min, temp_max, temp_step = 60.0, 100.0, 10.0
    temperatury = []
    curr_t = temp_min
    while curr_t <= temp_max:
        temperatury.append(round(curr_t, 1))
        curr_t += temp_step
    temperatury.reverse()  

    cool_min, cool_max, cool_step = 0.79, 0.99, 0.05
    wsp_chlodzenia = []
    curr_c = cool_min
    while curr_c <= cool_max:
        wsp_chlodzenia.append(round(curr_c, 3))
        curr_c += cool_step
    wsp_chlodzenia.reverse()  

    it = 15000
    ratings = [1.2, 1.5, 2.3, 2.6, 2.8, 3.0, 3.2, 3.4, 3.5, 3.6, 3.8, 4.0, 4.2, 4.4, 4.6, 4.8, 5.0, 5.2]
    
    # Obliczamy łączną liczbę kombinacji par (temp, cooling)
    total_combinations = len(temperatury) * len(wsp_chlodzenia)
    current_comb = 0
    
    print(f"\n--- URUCHAMIAM MULTIPROCESSING GRID SEARCH ---")
    print(f"Sprawdzam łącznie {total_combinations} kombinacji parametrów.")
    print(f"{'PROGRESS':<8} | {'TEMP':<6} | {'COOLING':<8} | {'ITERS':<7} | {'FINAL SE':<8} | {'ERROR':<15} | {'TIME':<6}")
    print("-" * 75)

    # Przygotowanie płaskiej listy zadań dla procesora
    zadania = []
    for t in temperatury:
        for c in wsp_chlodzenia:
            for r in ratings:
                zadania.append((t, c, it, r))
                
    # Struktura do grupowania wyników z procesów
    # klucz: (temp, cooling) -> wartość: lista wyników cząstkowych dla każdego ratingu
    slownik_grupujacy = {}
    
    najlepsze_params = None
    najmniejszy_blad_sredni = float('inf')
    
    # Uruchamiamy pulę procesów na 100% CPU
    with ProcessPoolExecutor(max_workers=None) as executor:
        przyszle_zadania = {
            executor.submit(sprawdz_pojedynczy_kontekst, t, c, i, r): (t, c, i, r) 
            for t, c, i, r in zadania
        }
        
        # Odbieramy wyniki pojedynczo, jak tylko procesor skończy dane podzadanie
        for future in as_completed(przyszle_zadania):
            data = future.result()
            
            # Tworzymy unikalny klucz dla danej pary parametrów
            klucz_grupy = (data['temp'], data['cooling'])
            
            if klucz_grupy not in slownik_grupujacy:
                slownik_grupujacy[klucz_grupy] = []
                
            # Dorzucamy wynik do odpowiedniej grupy
            slownik_grupujacy[klucz_grupy].append(data)
            
            # JEŻELI zebraliśmy już wszystkie ratingi (np. 18) dla tej konkretnej pary parametrów
            if len(slownik_grupujacy[klucz_grupy]) == len(ratings):
                current_comb += 1
                
                # Wyliczamy statystyki dokładnie tak, jak w Twojej funkcji sprawdz_parametry:
                error_avg = 0
                error_number = 0
                ostatni_rating = 0.0
                total_duration = 0.0
                
                for res in slownik_grupujacy[klucz_grupy]:
                    error_avg += res['error']
                    ostatni_rating = res['final_rating']  # Weźmie ostatni przetworzony
                    total_duration += res['duration']
                    if res['error'] > 0:
                        error_number += 1
                        
                blad = error_avg / len(ratings)
                procent_bledow = error_number / len(ratings)
                
                # WYŚWIETLANIE: Dokładnie Twój oczekiwany format wyjściowy w jednej linii!
                print(f"[{current_comb}/{total_combinations}] | {klucz_grupy[0]:<6.1f} | {klucz_grupy[1]:<8.3f} | {it:<7} | {ostatni_rating:<8.2f} | {blad:<7.2f}({procent_bledow:.2f}) | {total_duration:.1f}s")
                
                # Szukamy globalnego zwycięzcy
                if blad < najmniejszy_blad_sredni:
                    najmniejszy_blad_sredni = blad
                    najlepsze_params = {
                        'temp': klucz_grupy[0],
                        'cooling': klucz_grupy[1],
                        'iters': it,
                        'target': 3.0
                    }

    print("-" * 75)
    if najlepsze_params:
        print(f"NAJLEPSZA ZNALEZIONA KONFIGURACJA:")
        print(f" > Parametry: Temp={najlepsze_params['temp']}, Cooling={najlepsze_params['cooling']}, Iters={najlepsze_params['iters']}")
        print(f" > Średni najmniejszy błąd: {najmniejszy_blad_sredni:.2f}")
    
    return najlepsze_params

# =====================================================================
# Główny punkt wejścia (Main)
# =====================================================================
if __name__ == "__main__":
    # W systemach operacyjnych (szczególnie macOS i Windows) przy multiprocessing
    # CAŁY kod startowy MUSI znajdować się pod tym warunkiem if __name__ == "__main__":
    
    # najlepsze_wyliczone_params = optymalizuj_parametry()  
    
    print("\nOptymalizacja za pomocą procesów zakończona!")
    input("\nNaciśnij Enter, aby rozpocząć walidację zestawów tekstowych...")

    print("=================================================")
    print("      SUDOKU GENERATOR WRAPPER                   ")
    print("=================================================\n")
    
    testy = ['easy', 'medium', 'hard', 'expert']
    targets = [1.5, 2.5, 3.5, 5.0]
    temp = {'temp': 20.0, 'cooling': 0.99, 'iters': 1000000, 'target': 5.0}  # Przykładowe parametry (możesz je dostosować)
    for test in targets:
        print(f"Rozpoczynam generowanie dla: {test}...")
        start_time = time.time()
        
        temp['target'] = test
        wynik = generuj(temp)
        
        elapsed_time = time.time() - start_time
        
        print(f" > Plansza:       {wynik['final_puzzle']}")
        print(f" > Podpowiedzi:   {wynik['clues_count']}")
        print(f" > Oczekiwane SE: {wynik['target_rating']}")
        print(f" > Osiągnięte SE: {wynik['final_rating']}")
        print(f" > Czas operacji: {elapsed_time:.2f} s\n")
        print("-" * 49)
        
    print("Wszystkie testy zakończone!")