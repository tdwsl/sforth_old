#include "sforth.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **args) {
  if(argc > 2) {
    printf("usage: %s <file>\n", args[0]);
    return 0;
  }

  Forth *fth = forth_newForth();

  if(argc == 2) {
    forth_doFile(fth, (const char*)args[1]);
    forth_freeForth(fth);
    return 0;
  }

  printf("sforth - forth interpreter - tdwsl 2022\n");
  printf("type 'bye' to quit\n");

  int max = 80;
  char *s = (char*)malloc(max);
  int buf = 20;
  int len;

  while(!fth->quit) {
    len = 0;
    for(;;) {
      char c;
      scanf("%c", &c);
      //printf("%c (%d)\n", c, c);

      if(c == '\n' || c == 0 || c == 0x0a)
        break;

      s[len++] = c;

      if(len > max-buf) {
        max += buf;
        s = (char*)realloc(s, max);
      }
    }

    if(!len)
      continue;

    s[len] = 0;
    forth_doString(fth, s);

    if(forth_done(fth) && !fth->quit) {
      forth_printStack(fth);
      printf("    ok\n");
    }
  }

  printf("bye!\n");

  free(s);
  forth_freeForth(fth);

  return 0;
}
