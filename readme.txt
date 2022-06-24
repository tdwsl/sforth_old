sforth - a forth interpreter - tdwsl 2022

*** NOTE ***
I started this project when I didn't know anything about forth interpreters,
so now that I know a *little* more, I'll probably be working on a new one,
if that. Either way, I won't be commiting to this project, but feel free to
use this anyway. Just bear in mind that lots of stuff doesn't work as it
should.
***

sforth is a Forth interpreter that can be embedded within a C program using
a simple API. It should work on most architectures. Some words may be
missing, but it's easy to define custom words in C. An interpreter program
is included. That being said, its functionality is quite limited, as it
doesn't support more advanced features.

Here is an example of using sforth to add a function and run a file:

#include <sforth.h>

void f_sq(Forth *fth) {
  int n = forth_pop(fth);
  forth_push(fth, n*n);
}

int main() {
  Forth *fth = forth_newForth();
  forth_addFunction(fth, f_sq, "SQ");
  forth_doFile(fth, "test.fth");
  forth_freeForth(fth);
  return 0;
}

When compiling, use -lsforth to link to the library once it is installed.

For more information on how to use the API, check out the 'examples' folder.

To compile, all you really need is a C compiler. To use the Makefile, you'll
need make and binutils (for installing the static library). Check the
Makefile for more information on compiling.

This software is licensed under the MIT license. For more information, look
at 'license.txt'.

