/* guess a random number from 0-9 */

#include "sforth.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

void f_rand(Forth *fth) {
  int i = rand();
  forth_push(fth, i);
}

int main() {
  Forth *fth = forth_newForth();
  forth_addFunction(fth, f_rand, "RAND");
  srand(time(0));

  forth_doString(fth, ""
"RAND 10 MOD CONSTANT N "
".( Guess a number from 0-9) CR "
": GUESS "
"  KEY 48 - "
"  DUP -1 > OVER 10 < AND IF "
"    DUP N <> IF "
"      DUP N > IF .( Too high) "
"      ELSE .( Too low) THEN CR "
"    THEN "
"  THEN "
"; "
"BEGIN GUESS N = UNTIL "
".( You guessed it!) CR "
".( The number was ) N . CR ");

  forth_freeForth(fth);
  return 0;
}
