EXEC = primecount

all: $(EXEC)

primecount: primecount.o
	gcc primecount.o -o primecount -lm -pthread

primecount.o: primecount.c
	gcc -c primecount.c
	
clean:
	rm -f *.o $(EXEC)
