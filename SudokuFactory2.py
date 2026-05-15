import time
import random
from src.SimulatedAnnealing.SASudoku import SA_SudokuGenerator
from src.Rating.Rater import Rate_sudoku

def generate_sudoku_factory(base_solutions, targets):
    """
    Zarządca generowania Sudoku o różnych trudnościach.
    
    :param base_solutions: Lista rozwiązanych plansz (81 znaków)
    :param targets: Lista słowników z parametrami (iteracje, temp, nazwa)
    """
    print("="*50)
    print("       SUDOKU FACTORY - GENERATOR SERYJNY")
    print("="*50)
    
    generated_puzzles = []
    
    for i, config in enumerate(targets):
        start_time = time.time()
        # Wybieramy losową bazę dla każdej zagadki
        base = random.choice(base_solutions)
        
        print(f"\n[PRODUKCJA] Zagadka #{i+1}: {config['label']}")
        print(f" -> Parametry: Temp={config['temp']}, Chłodzenie={config['rate']}, Iteracje={config['iters']}")
        
        # Inicjalizacja generatora
        generator = SA_SudokuGenerator(
            solved_board=base,
            initialTemperature=config['temp'],
            coolingRate=config['rate'],
            iterations=config['iters']
        )
        
        # Uruchomienie procesu ewolucji
        best_mask = generator.run()
        
        
        # Finalna obróbka
        final_puzzle = generator.apply_mask_to_str(best_mask)
        final_rating = -generator.cost(best_mask) # Koszt to -rating, więc odwracamy
        clues = sum(best_mask)
        my_rating = Rate_sudoku(final_puzzle)
        
        elapsed = time.time() - start_time
        
        print(f" -> Sukces! Czas: {elapsed:.2f}s")
        print(f" -> Wynik końcowy: SE {final_rating:.1f} | Podpowiedzi: {clues} | Grader: {my_rating:.1f}")
        print(f" -> Plansza: {final_puzzle}")
        
        generated_puzzles.append({
            'label': config['label'],
            'puzzle': final_puzzle,
            'rating': final_rating,
            'clues': clues
        })
        
    return generated_puzzles

if __name__ == "__main__":
    # Przykładowe rozwiązane plansze (Twoja baza)
    my_bases = [
        "435269781682571493197834562826195347374682915951743628519326874248957136763418259" # Przykładowy string 81 znaków
    ]
    
    # Definiujemy co chcemy osiągnąć
    production_plan = [
        {"label": "Szybki Easy",   "temp": 10.0, "rate": 0.99,  "iters": 100},
        {"label": "Solidny Medium", "temp": 20.0, "rate": 0.995, "iters": 500},
        {"label": "Ciężki Hard",    "temp": 50.0, "rate": 0.999, "iters": 2000},
        {"label": "Extreme Search", "temp": 100.0, "rate": 0.9995, "iters": 5000}
    ]
    
    results = generate_sudoku_factory(my_bases, production_plan)
    
    # Zapis do pliku wynikowego
    with open("generated_batch.txt", "w") as f:
        for res in results:
            f.write(f"{res['label']} | SE: {res['rating']:.1f} | Clues: {res['clues']} | {res['puzzle']}\n")
    
    print(f"\n[FINISH] Wygenerowano {len(results)} zagadek. Wyniki zapisano w generated_batch.txt")