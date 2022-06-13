sforth - a forth interpreter - tdwsl 2022

sforth is a forth interpreter with an easy-to-use API. It should work on
most architectures. It compiles each instruction down to bytecode, so
conditionals and loops work outside of words. An interpreter is included.

Here is an example of using the library to run a file:

#include <sforth.h>

int main() {
  Forth *fth = forth_newForth();
  forth_doFile(fth, "test.fth");
  forth_freeForth(fth);
  return 0;
}

When compiling, use -lsforth to link to the library once it is installed.

For information on how to use the API, check out the 'examples' folder.

To compile, all you really need is a C compiler. To use the Makefile, you'll
need make and binutils (for installing the static library). Check the
Makefile for more information on compiling.

This software is licensed under the MIT license. For more information, look
at 'license.txt'.
