CC = gcc

all: main

main: bunedufork.o
	$(CC) -o bunedufork bunedufork.o
	
bunedufork.o: bunedufork.c
	$(CC) -c bunedufork.c
	
clean:
	rm -f *.o all 