#include "sforth.h"
#include <stdio.h>
#include <stdint.h>

void forth_keyFunction(Forth *fth) {
  char c = fgetc(stdin);
  while(fgetc(stdin) != '\n');

  forth_push(fth, (void*)(intmax_t)c);
}

void forth_addDefaultWords(Forth *fth) {
  forth_addWord(fth, ">=");
  forth_addInstruction(fth, FORTH_INC);
  forth_addInstruction(fth, FORTH_GREATER);
  forth_addInstruction(fth, FORTH_RET);

  forth_addWord(fth, "<=");
  forth_addInstruction(fth, FORTH_DEC);
  forth_addInstruction(fth, FORTH_LESS);
  forth_addInstruction(fth, FORTH_RET);

  forth_addWord(fth, "=");
  forth_addInstruction(fth, FORTH_EQUAL);
  forth_addInstruction(fth, FORTH_RET);

  forth_addWord(fth, "<");
  forth_addInstruction(fth, FORTH_LESS);
  forth_addInstruction(fth, FORTH_RET);

  forth_addWord(fth, ">");
  forth_addInstruction(fth, FORTH_GREATER);
  forth_addInstruction(fth, FORTH_RET);

  forth_addWord(fth, "1-");
  forth_addInstruction(fth, FORTH_DEC);
  forth_addInstruction(fth, FORTH_RET);

  forth_addWord(fth, "1+");
  forth_addInstruction(fth, FORTH_INC);
  forth_addInstruction(fth, FORTH_RET);

  forth_addWord(fth, "CR");
  forth_addInstruction(fth, FORTH_CR);
  forth_addInstruction(fth, FORTH_RET);

  forth_addWord(fth, ".");
  forth_addInstruction(fth, FORTH_PRINT);
  forth_addInstruction(fth, FORTH_RET);

  forth_addWord(fth, "2DUP");
  forth_addInstruction(fth, FORTH_OVER);
  forth_addInstruction(fth, FORTH_OVER);
  forth_addInstruction(fth, FORTH_RET);

  forth_addWord(fth, "ROT");
  forth_addInstruction(fth, FORTH_ROT);
  forth_addInstruction(fth, FORTH_RET);

  forth_addWord(fth, "SWAP");
  forth_addInstruction(fth, FORTH_SWAP);
  forth_addInstruction(fth, FORTH_RET);

  forth_addWord(fth, "OVER");
  forth_addInstruction(fth, FORTH_OVER);
  forth_addInstruction(fth, FORTH_RET);

  forth_addWord(fth, "DUP");
  forth_addInstruction(fth, FORTH_DUP);
  forth_addInstruction(fth, FORTH_RET);

  forth_addWord(fth, "DROP");
  forth_addInstruction(fth, FORTH_DROP);
  forth_addInstruction(fth, FORTH_RET);

  forth_addWord(fth, "MOD");
  forth_addInstruction(fth, FORTH_MOD);
  forth_addInstruction(fth, FORTH_RET);

  forth_addWord(fth, "/");
  forth_addInstruction(fth, FORTH_DIV);
  forth_addInstruction(fth, FORTH_RET);

  forth_addWord(fth, "*");
  forth_addInstruction(fth, FORTH_MUL);
  forth_addInstruction(fth, FORTH_RET);

  forth_addWord(fth, "-");
  forth_addInstruction(fth, FORTH_MINUS);
  forth_addInstruction(fth, FORTH_RET);

  forth_addWord(fth, "+");
  forth_addInstruction(fth, FORTH_PLUS);
  forth_addInstruction(fth, FORTH_RET);

  forth_addWord(fth, "/MOD");
  forth_addInstruction(fth, FORTH_OVER);
  forth_addInstruction(fth, FORTH_OVER);
  forth_addInstruction(fth, FORTH_MOD);
  forth_addInstruction(fth, FORTH_ROT);
  forth_addInstruction(fth, FORTH_ROT);
  forth_addInstruction(fth, FORTH_DIV);
  forth_addInstruction(fth, FORTH_RET);

  forth_addWord(fth, "HERE");
  forth_addInstruction(fth, FORTH_HERE);
  forth_addInstruction(fth, FORTH_RET);

  forth_addWord(fth, "ALLOT");
  forth_addInstruction(fth, FORTH_ALLOT);
  forth_addInstruction(fth, FORTH_RET);

  forth_addWord(fth, "@");
  forth_addInstruction(fth, FORTH_GETMEM);
  forth_addInstruction(fth, FORTH_RET);

  forth_addWord(fth, "!");
  forth_addInstruction(fth, FORTH_SETMEM);
  forth_addInstruction(fth, FORTH_RET);

  forth_addWord(fth, "BYE");
  forth_addInstruction(fth, FORTH_BYE);
  forth_addInstruction(fth, FORTH_RET);

  forth_addWord(fth, "AND");
  forth_addInstruction(fth, FORTH_AND);
  forth_addInstruction(fth, FORTH_RET);

  forth_addWord(fth, "OR");
  forth_addInstruction(fth, FORTH_OR);
  forth_addInstruction(fth, FORTH_RET);

  forth_addWord(fth, "XOR");
  forth_addInstruction(fth, FORTH_XOR);
  forth_addInstruction(fth, FORTH_RET);

  forth_addWord(fth, "NOT");
  forth_addInstruction(fth, FORTH_NOT);
  forth_addInstruction(fth, FORTH_RET);

  forth_addWord(fth, "INVERT");
  forth_addInstruction(fth, FORTH_INVERT);
  forth_addInstruction(fth, FORTH_RET);

  forth_addWord(fth, "EMIT");
  forth_addInstruction(fth, FORTH_EMIT);
  forth_addInstruction(fth, FORTH_RET);

  forth_addWord(fth, "KEY");
  forth_addInstruction(fth, FORTH_FUNCTION);
  forth_addValue(fth, (void*)forth_keyFunction);
  forth_addInstruction(fth, FORTH_RET);

  forth_addWord(fth, "PRINTPROGRAM");
  forth_addInstruction(fth, FORTH_PRINTPROGRAM);
  forth_addInstruction(fth, FORTH_RET);

  forth_addWord(fth, "TRACEON");
  forth_addInstruction(fth, FORTH_TRACEON);
  forth_addInstruction(fth, FORTH_RET);

  forth_addWord(fth, "TRACEOFF");
  forth_addInstruction(fth, FORTH_TRACEOFF);
  forth_addInstruction(fth, FORTH_RET);

  fth->old_size = fth->size;
}
