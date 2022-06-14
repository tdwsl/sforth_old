CC=cc
CFLAGS=-O2 -Wall -I..
ARGS=$(CFLAGS) ../sforth.o

default:
	$(CC) $(ARGS) guess.c -o guess
	$(CC) $(ARGS) cipher.c -o cipher

clean:
	rm -f guess cipher
