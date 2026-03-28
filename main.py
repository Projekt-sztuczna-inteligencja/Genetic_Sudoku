from src.Board import Board
from src.Genetic.GeneticSudoku import GeneticSudoku
from src.Genetic.FastSudoku import FastSudoku
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



  gen = GeneticSudoku(populationSize=1000, mutationRate=0.6, generations=100000, eliteSize=50,
                crossoverFunctionName="square")
  
  gen = FastSudoku(populationSize=2000, mutationRate=0.6, generations=100000, eliteSize=50, crossoverFunctionName="square")

  winner = gen.run()
  print("Winner:")
  print(winner)


main()