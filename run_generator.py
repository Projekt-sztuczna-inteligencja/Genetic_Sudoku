import concurrent.futures
import time
from multiprocessing import Manager
from typing import get_args

from src.Genetic.SudokuEngine import FastSudoku
from src.Genetic.CrossoverMethod import crossoverType
from src.SudokuBoard import Board


def run_island(island_id: int, stop_event):
    """Zadanie wykonywane przez izolowany proces - uruchamia osobną instancję algorytmu."""
    print(f"🏝️ Wyspa {island_id} startuje...")

    available_crossovers = list(get_args(crossoverType))
    crossover = available_crossovers[island_id % len(available_crossovers)]

    gen = FastSudoku(
        populationSize=2000,
        mutationRate=0.6,
        generations=100000,
        eliteSize=50,
        crossoverFunctionName=crossover
    )

    # Odpalamy algorytm przekazując mu flagę
    winner = gen.run(stop_event)

    # Jeśli wynik to None, znaczy że ten proces został zabity przez inną wyspę
    if winner is None:
        return None

    # Wygrywamy, wciskamy przycisk awaryjny żeby ubić resztę wysp
    stop_event.set()
    return island_id, winner, crossover


def main():
    num_islands = 4
    print(f"🚀 Odpalamy wyścig na {num_islands} rdzeniach!\n")
    start_time = time.time()

    # Manager pozwala na komunikację flagami między procesami CPU
    with Manager() as manager:
        stop_event = manager.Event()

        with concurrent.futures.ProcessPoolExecutor(max_workers=num_islands) as executor:
            futures = [executor.submit(run_island, i, stop_event) for i in range(num_islands)]

            for future in concurrent.futures.as_completed(futures):
                result = future.result()

                # Reagujemy na wynik pierwszego procesu, który wrócił ze zwycięstwem
                if result is not None:
                    island_id, winner, c = result
                    elapsed = time.time() - start_time

                    print("\n" + "=" * 40)
                    print(f"🏆 WYŚCIG ZAKOŃCZONY!")
                    print(f"Wyspa {island_id} znalazła rozwiązanie w {elapsed:.2f} sekund!")
                    print("=" * 40)
                    print("Rozwiązana plansza:")
                    print(Board(winner))
                    print(f"Użyta metoda krzyżowania: {c}\n")

                    # Nakaz wymuszonego ubicia reszty zadań
                    executor.shutdown(wait=False, cancel_futures=True)
                    break


if __name__ == '__main__':
    main()