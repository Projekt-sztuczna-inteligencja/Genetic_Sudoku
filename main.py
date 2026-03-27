from src.Board import Board
from src.Genetic import Genetic
def main():
  board = Board()
  for y in range(9):
    for x in range(9):
      board.write(x, y, str((y * 3 + x) % 9 + 1))
  sq1 = board.get_square(0)
  for cell in sq1:
    print(cell)
  print(board)
  
  test = Board()
  test.random()
  sq1 = test.get_square(0)
  print(test)



  gen = Genetic(population_size=200, mutation_rate=0.6, generations=100000, elite_size=10)
  gen.run()

main()