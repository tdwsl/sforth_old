sforth - a forth interpreter - tdwsl 2022

sforth is a Forth interpreter that can be embedded within a C program using
a simple API. It should work on most architectures. It doesn't support
advanced features such as word arguments, and some words may be missing,
but it's easy to define custom words in C. An interpreter program is
included.

Here is an example of using sforth to run a file:

#include <sforth.h>

int main() {
  Forth *fth = forth_newForth();
  forth_doFile(fth, "test.fth");
  forth_freeForth(fth);
  return 0;
}

When compiling, use -lsforth to link to the library once it is installed.

sforth comes with an API for defining words in C. For information on how to
use the API, check out the 'examples' folder.

To compile, all you really need is a C compiler. To use the Makefile, you'll
need make and binutils (for installing the static library). Check the
Makefile for more information on compiling.

This software is licensed under the MIT license. For more information, look
at 'license.txt'.

