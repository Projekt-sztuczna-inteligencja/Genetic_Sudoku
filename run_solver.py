import concurrent.futures
import time
from multiprocessing import Manager
from typing import get_args

from src.Genetic.SudokuEngine import FastSudoku
from src.Genetic.CrossoverMethod import crossoverType
from src.SudokuBoard import Board


def transform_to_squares(puzzle: list[int]) -> list[int]:
    """
    Tłumacz DNA: Przerabia klasyczną planszę (czytaną wierszami) na wewnętrzną
    reprezentację algorytmu (bloki 9 elementów = kwadraty 3x3).
    Dzięki temu święte liczby trafiają do odpowiednich genów.
    """
    sq_puzzle = [0] * 81
    for r in range(9):
        for c in range(9):
            sq_idx = (r // 3) * 27 + (c // 3) * 9 + (r % 3) * 3 + (c % 3)
            sq_puzzle[sq_idx] = puzzle[r * 9 + c]
    return sq_puzzle


def run_island(island_id: int, stop_event):
    """Uruchamia pojedynczy proces (wyspę) rozwiązujący Sudoku."""
    print(f"🏝️ Wyspa {island_id} startuje...")

    # Ekstremalne Sudoku (17 wskazówek)
    sample_puzzle = [
        0, 0, 0, 0, 0, 0, 0, 1, 0,
        4, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 2, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 5, 0, 4, 0, 7,
        0, 0, 8, 0, 0, 0, 3, 0, 0,
        0, 0, 1, 0, 9, 0, 0, 0, 0,
        3, 0, 0, 4, 0, 0, 2, 0, 0,
        0, 5, 0, 1, 0, 0, 0, 0, 0,
        0, 0, 0, 8, 0, 6, 0, 0, 0
    ]

    genetic_puzzle = transform_to_squares(sample_puzzle)

    # Dynamiczne pobranie dostępnych metod krzyżowania
    available_crossovers = list(get_args(crossoverType))
    crossover = available_crossovers[island_id % len(available_crossovers)]

    gen = FastSudoku(
        populationSize=2000,
        mutationRate=0.6,
        generations=100000,
        eliteSize=100,
        crossoverFunctionName=crossover,
        initial_board=genetic_puzzle
    )

    winner = gen.run(stop_event)

    # Proces został ubity przez inne wyspy
    if winner is None:
        return None

    # Ten proces wygrał - blokujemy flagą resztę wątków
    stop_event.set()
    return island_id, winner, crossover


def main():
    num_islands = 4
    print(f"\n🚀 Odpalamy wyścig ewolucyjny na {num_islands} rdzeniach!\n")
    start_time = time.time()

    with Manager() as manager:
        stop_event = manager.Event()

        with concurrent.futures.ProcessPoolExecutor(max_workers=num_islands) as executor:
            futures = [executor.submit(run_island, i, stop_event) for i in range(num_islands)]

            for future in concurrent.futures.as_completed(futures):
                result = future.result()

                if result is not None:
                    island_id, winner, crossover = result
                    elapsed = time.time() - start_time

                    print("\n" + "=" * 50)
                    print(f"🏆 WYŚCIG ZAKOŃCZONY!")
                    print(f"Wyspa {island_id} znalazła rozwiązanie w {elapsed:.2f} sekund!")
                    print("=" * 50)

                    print("\nRozwiązana plansza:")
                    print(Board(winner))
                    print(f"Użyta metoda krzyżowania: {crossover}\n")

                    executor.shutdown(wait=False, cancel_futures=True)
                    break


if __name__ == '__main__':
    main()