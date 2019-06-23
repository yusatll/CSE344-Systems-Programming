CC = gcc

all: main

main: bunedu.o
	$(CC) -o bunedu bunedu.o
	
bunedu.o: bunedu.c
	$(CC) -c bunedu.c
	
clean:
	rm -f *.o all 