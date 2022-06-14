CC=cc
CFLAGS=-O2 -Wall

sforth: sforth.c sforth.h interpreter.c
	$(CC) $(CFLAGS) -c sforth.c
	$(CC) $(CFLAGS) sforth.o interpreter.c -o sforth

install: sforth
	ar ruv libsforth.a sforth.o
	ranlib libsforth.a
	cp sforth /usr/bin
	cp libsforth.a /usr/lib
	cp sforth.h /usr/include

uninstall:
	rm /usr/bin/sforth
	rm /usr/lib/libsforth.a
	rm /usr/include/sforth.h

clean:
	rm -f libsforth.a sforth.o sforth
