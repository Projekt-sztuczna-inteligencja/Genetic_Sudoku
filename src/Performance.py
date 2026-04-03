import multiprocessing
import time
from typing import Any

class Performance:
    def __init__(self, algorithms: list[Any], nrProcesses: int):
        self.algorithms = algorithms
        self.nrProcesses = nrProcesses
        self.start_time = None
        self.end_time = None

    @staticmethod
    def _runProcess(idx: int, algo: Any, queue: multiprocessing.Queue):
        print(f"Process {idx} started...")
        
        try:
            # Run the algorithm
            winner = algo.run()
            # Pass the result to the main process
            queue.put((idx, winner))
        except Exception as e:
            print(f"Error in process {idx}: {e}")

    def run(self):
        print(f"Started computing (processes = {self.nrProcesses})")
        self.start_time = time.time()

        queue = multiprocessing.Queue()
        processes = []

        # 1. Initialize and start processes
        for i in range(self.nrProcesses):
            algo_instance = self.algorithms[i]
            p = multiprocessing.Process(target=self._runProcess, args=(i, algo_instance, queue))
            p.start()
            processes.append(p)

        # 2. Wait for the first result in the queue
        idx, winner = queue.get()
        
        self.end_time = time.time()
        elapsed = self.end_time - self.start_time
        
        print("\n" + "="*50)
        print("COMPUTATION FINISHED")
        print(f"Process  {idx} found a solution in {elapsed:.2f} s.")
        print("="*50)
        
        # Print the result
        print(winner)

        # 3. Force termination of the remaining processes
        print("\nStopping background processes...")
        for p in processes:
            if p.is_alive():
                p.terminate()
                p.join()
        print("All processes have been successfully terminated.")