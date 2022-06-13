#include "sforth.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

void forth_printStack(Forth *fth) {
  printf("<");
  for(int i = 0; i < fth->sp; i++) {
    printf("%jd", (intmax_t)(fth->stack[i]));
    if(i < fth->sp-1)
      printf(" ");
  }
  printf(">");
}

int forth_done(Forth *fth) {
  if(fth->if_sp
      || fth->begin_sp
      || fth->do_sp)
    return 0;

  if(fth->mode != FORTHMODE_NORMAL)
    return 0;

  return 1;
}

int forth_isInteger(char *s, intmax_t *n) {
  if(strlen(s) < 1)
    return 0;

  *n = 0;
  int neg = 0;

  for(int i = 0; s[i]; i++) {
    if(s[i] == '-') {
      if(i == 0 && s[i+1])
        neg = 1;
      else
        return 0;
    }
    else if(s[i] >= '0' && s[i] <= '9')
      *n = (*n)*10 + s[i]-'0';
    else
      return 0;
  }

  if(neg)
    *n *= -1;
  return 1;
}

int forth_validIdentifier(char *s) {
  if(strlen(s) < 1)
    return 0;
  intmax_t n;
  if(forth_isInteger(s, &n))
    return 0;
  for(int i = 0; s[i]; i++)
    if(s[i] <= ' ')
      return 0;
  const char *reserved[] = {
    ":",";",".\"",".(",".'",
    "I","IF","DO","THEN","ELSE","DO","LOOP","UNTIL","BEGIN",
    "CREATE","FORGET","VARIABLE","CONSTANT", "INCLUDE",
    0,
  };
  for(int i = 0; reserved[i]; i++)
    if(strcmp(s, reserved[i]) == 0)
      return 0;
  return 1;
}

void forth_defaultEmit(char c) {
  printf("%c", c);
}

Forth *forth_newForth() {
  Forth *fth = (Forth*)malloc(sizeof(Forth));

  fth->sp = 0;

  fth->here = (void*)fth->memory;

  fth->size = 0;
  fth->old_size = 0;

  fth->num_words = 0;

  fth->if_sp = 0;
  fth->do_sp = 0;
  fth->begin_sp = 0;

  fth->mode = FORTHMODE_NORMAL;

  fth->quit = 0;

  fth->trace = 0;

  fth->emit = forth_defaultEmit;

  forth_addDefaultWords(fth);

  return fth;
}

void forth_freeForth(Forth *fth) {
  /* free strings */
  int pc = 0;
  while(pc < fth->size) {
    switch(fth->program[pc]) {
    case FORTH_PRINTSTRING:
    case FORTH_CREATE:
    case FORTH_VARIABLE:
    case FORTH_CONSTANT:
    case FORTH_FORGET:
    case FORTH_INCLUDE:
      free(forth_getValue(fth, pc+1));
      break;
    }
    forth_nextInstruction(fth, &pc);
  }

  /* free identifiers */
  for(int i = 0; i < fth->num_words; i++)
    free(fth->words[i].name);

  free(fth);
}

int forth_has(Forth *fth, int n) {
  if(fth->sp >= n)
    return 1;
  else {
    printf(FORTH_UNDERFLOW_ERR);
    return 0;
  }
}

void *forth_pop(Forth *fth) {
  if(forth_has(fth, 1))
    return fth->stack[--(fth->sp)];
  else
    return 0;
}

void forth_push(Forth *fth, void *val) {
  fth->stack[fth->sp++] = val;
}
