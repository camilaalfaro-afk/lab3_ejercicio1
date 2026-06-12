CC = gcc
CFLAGS = -Wall -Wextra -O0

all: ejercicio1 ejercicio2

ejercicio1: ejercicio1.c
	$(CC) $(CFLAGS) ejercicio1.c -o ejercicio1

ejercicio2: ejercicio2.c
	$(CC) $(CFLAGS) ejercicio2.c -o ejercicio2

clean:
	rm -f ejercicio1 ejercicio2
