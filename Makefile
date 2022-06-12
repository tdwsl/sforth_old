CC=cc
CFLAGS=-O2 -Wall

sforth: sforth.h sforth_run.c sforth_word.c sforth_parse.c sforth_general.c sforth_library.c sforth_compile.c
	$(CC) $(CFLAGS) -c sforth_run.c sforth_word.c sforth_parse.c sforth_general.c sforth_library.c sforth_compile.c
	$(CC) $(CFLAGS) *.o interpreter.c -o sforth

install: sforth
	ar ruv libsforth.a *.o
	ranlib libsforth.a
	cp sforth /usr/bin
	cp libsforth.a /usr/lib
	cp sforth.h /usr/include

uninstall:
	rm /usr/bin/sforth
	rm /usr/lib/libsforth.a
	rm /usr/include/sforth.h

clean:
	rm -f libsforth.a *.o sforth
