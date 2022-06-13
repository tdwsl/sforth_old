CC=cc
CFLAGS=-O2 -Wall -I..
OBJS=../*.o
ARGS=$(CFLAGS) $(OBJS)

default:
	$(CC) $(ARGS) guess.c -o guess
	$(CC) $(ARGS) cipher.c -o cipher

clean:
	rm -f guess cipher
