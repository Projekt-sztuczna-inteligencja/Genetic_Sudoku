import concurrent.futures
import time
from src.Genetic.FastSudoku import FastSudoku
from src.Genetic.CrossoverFast import crossoverType

# 1. Tworzymy funkcję, którą każdy proces odpali u siebie
def run_island(island_id: int):
    print(f"🏝️ Wyspa {island_id} startuje...")
    available_crossovers = [t for t in crossoverType]
    c = available_crossovers[island_id % len(available_crossovers)]  # Wybieramy crossover na podstawie ID wyspy (modulo liczby dostępnych metod)
    # Inicjalizujemy instancję algorytmu
    gen = FastSudoku(
        populationSize=2000, 
        mutationRate=0.6, 
        generations=100000, 
        eliteSize=50, 
        crossoverFunctionName= c
    )
    
    # Odpalamy algorytm!
    winner = gen.run()
    
    return island_id, winner, c

def main():
    # Zostawiam Twoje testowe kody dla Board (możesz je usunąć jeśli chcesz)
    # ...
    
    num_islands = 4  # Zależy od liczby rdzeni w Twoim procesorze (np. 4 lub 8)
    
    print(f"🚀 Odpalamy wyścig na {num_islands} rdzeniach!")
    start_time = time.time()
    
    # 2. Używamy ProcessPoolExecutor do odpalenia fizycznych procesów
    with concurrent.futures.ProcessPoolExecutor(max_workers=num_islands) as executor:
        
        # Rozdzielamy zadania do procesów
        futures = [executor.submit(run_island, i) for i in range(num_islands)]
        
        # 3. Czekamy na PIERWSZEGO, który skończy
        for future in concurrent.futures.as_completed(futures):
            island_id, winner, c = future.result()
            
            elapsed = time.time() - start_time
            print("\n" + "="*40)
            print(f"🏆 WYŚCIG ZAKOŃCZONY!")
            print(f"Wyspa {island_id} znalazła rozwiązanie w {elapsed:.2f} sekund!")
            print("="*40)
            print(winner)
            print(f"Użyta metoda krzyżowania: {c}")
            
            # Ubijamy pozostałe procesy, żeby nie zużywały procesora
            executor.shutdown(wait=False, cancel_futures=True)
            break 

if __name__ == '__main__':
    main()