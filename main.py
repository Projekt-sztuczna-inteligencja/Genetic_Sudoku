from src.Board import Board
from src.Performance import Performance
from src.Genetic.rowBased.GnRowBased import GnRowBased
from src.Genetic.rowBased.Crossover import crossoverType

def main():
    nrProcesses = 3
    TestObjects= []
    for i in range(nrProcesses):
        crType = crossoverType.__args__[i % len(crossoverType.__args__)]
        obj = GnRowBased(populationSize=1000, mutationRate=0.5, generations=10000, 
                        eliteSize=50, crossoverFunctionName=crType)
        TestObjects.append(obj)
    
    Test = Performance(TestObjects, nrProcesses)
    Test.run()

if __name__ == '__main__':
    main()

    