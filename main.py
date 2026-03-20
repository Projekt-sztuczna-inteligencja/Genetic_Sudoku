import src.Board
import src.Genetic
def main():
  board = src.Board.Board()
  for y in range(9):
    for x in range(9):
      board.write(x, y, str((y * 3 + x) % 9 + 1))
  sq1 = board.get_square(0)
  for cell in sq1:
    print(cell)
  print(board)
  gen = src.Genetic.Genethic(2, 0.01, 100000)
  gen.run()

main()