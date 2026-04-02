import concurrent.futures
import time
from multiprocessing import Manager
from src.Genetic.FastSudoku import FastSudoku
from src.Genetic.CrossoverFast import crossoverType
from src.Board import Board  # <-- Dodane do ładnego rysowania planszy
from typing import get_args

def transform_to_squares(puzzle):
    #Przerabia klasyczną plansze na DNA algorytmu
    sq_puzzle = [0]*81
    for r in range(9):
        for c in range(9):
            sq_idx = (r//3)*27 + (c//3)*9 + (r%3)*3 + (c%3)
            sq_puzzle[sq_idx] = puzzle[r*9 + c]
    return sq_puzzle

# 1. Tworzymy funkcję, którą każdy proces odpali u siebie
def run_island(island_id: int, stop_event):
    print(f"🏝️ Wyspa {island_id} startuje...")

    # Ekstremalne Sudoku z absolutnym minimum wskazówek (17 cyfr)
    # Najtrudniejsze Sudoku świata wg Arto Inkali (21 wskazówek)
    sample_puzzle = [
        8, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 3, 6, 0, 0, 0, 0, 0,
        0, 7, 0, 0, 9, 0, 2, 0, 0,
        0, 5, 0, 0, 0, 7, 0, 0, 0,
        0, 0, 0, 0, 4, 5, 7, 0, 0,
        0, 0, 0, 1, 0, 0, 0, 3, 0,
        0, 0, 1, 0, 0, 0, 0, 6, 8,
        0, 0, 8, 5, 0, 0, 0, 1, 0,
        0, 9, 0, 0, 0, 0, 4, 0, 0
    ]
    genetic_puzzle = transform_to_squares(sample_puzzle)

    available_crossovers = list(get_args(crossoverType))
    c = available_crossovers[island_id % len(available_crossovers)]

    gen = FastSudoku(
        populationSize=2000,
        mutationRate=0.6,
        generations=100000,
        eliteSize=50,
        crossoverFunctionName=c,
        initial_board=genetic_puzzle
    )

    # Odpalamy algorytm przekazując mu flagę!
    winner = gen.run(stop_event)

    # Jeśli wynik to None, znaczy że ten proces został zabity przez inną wyspę
    if winner is None:
        return None

    # Jeśli to MY wygraliśmy, wciskamy przycisk awaryjny żeby ubić resztę wysp!
    stop_event.set()

    return island_id, winner, c


def main():
    num_islands = 4

    print(f"🚀 Odpalamy wyścig na {num_islands} rdzeniach!")
    start_time = time.time()

    # Manager zarządza pamięcią RAM między wieloma rdzeniami procesora
    with Manager() as manager:
        stop_event = manager.Event()  # Współdzielona flaga (0 / 1)

        with concurrent.futures.ProcessPoolExecutor(max_workers=num_islands) as executor:

            # Przekazujemy stop_event do każdej wyspy
            futures = [executor.submit(run_island, i, stop_event) for i in range(num_islands)]

            for future in concurrent.futures.as_completed(futures):
                result = future.result()

                if result is not None:  # Ktoś wrócił ze zwycięstwem!
                    island_id, winner, c = result

                    elapsed = time.time() - start_time
                    print("\n" + "=" * 40)
                    print(f"🏆 WYŚCIG ZAKOŃCZONY!")
                    print(f"Wyspa {island_id} znalazła rozwiązanie w {elapsed:.2f} sekund!")
                    print("=" * 40)

                    # ŁADNE WYŚWIETLANIE ZWYCIĘSKIEJ PLANSZY
                    pretty_board = Board(winner)
                    print("Rozwiązana plansza:")
                    print(pretty_board)

                    print(f"Użyta metoda krzyżowania: {c}")

                    executor.shutdown(wait=False, cancel_futures=True)
                    break


if __name__ == '__main__':
    main()