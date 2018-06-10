all: main

main: main.o
	gcc -g -Wall -o myELF main.o

main.o: main.c
	gcc -g -Wall -c -o main.o main.c

.PHONY: clean

clean: 
	rm -f *.o myELF
