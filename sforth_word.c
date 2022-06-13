#include "sforth.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void forth_addWord(Forth *fth, char *name) {
  fth->words[fth->num_words].name = (char*)malloc(strlen(name)+1);
  strcpy(fth->words[fth->num_words].name, name);
  fth->words[fth->num_words++].addr = fth->size;
}

void forth_addInstruction(Forth *fth, char ins) {
  fth->program[fth->size++] = ins;
}

void forth_setValueIndex(Forth *fth, int addr, int i) {
  fth->program[addr] = (char)(i >> 8);
  fth->program[addr+1] = (char)(i & 0xff);
}

void forth_addValue(Forth *fth, void *val) {
  fth->values[fth->num_values++] = val;
  forth_setValueIndex(fth, fth->size, fth->num_values-1);
  fth->size += 2;
}

void forth_addString(Forth *fth, char *s) {
  char *str = (char*)malloc(strlen(s)+1);
  strcpy(str, s);
  forth_addValue(fth, (void*)str);
}

int forth_getValueIndex(Forth *fth, int addr) {
  int i = ((int)fth->program[addr])<<8 | fth->program[addr+1];
  return i;
}

void *forth_getValue(Forth *fth, int addr) {
  return fth->values[forth_getValueIndex(fth, addr)];
}

void forth_setValue(Forth *fth, int addr, void *val) {
  fth->values[forth_getValueIndex(fth, addr)] = val;
}

void forth_nextInstruction(Forth *fth, int *pc) {
  switch(fth->program[*pc]) {
  case FORTH_JUMP:
  case FORTH_JZ:
  case FORTH_LOOP:
  case FORTH_CALL:
  case FORTH_PUSH:
  case FORTH_PRINTSTRING:
  case FORTH_CREATE:
  case FORTH_VARIABLE:
  case FORTH_CONSTANT:
  case FORTH_FORGET:
  case FORTH_FUNCTION:
  case FORTH_INCLUDE:
    *pc += 3;
    break;
  default:
    (*pc)++;
  }
}

int forth_findWord(Forth *fth, char *name) {
  for(int i = 0; i < fth->num_words; i++)
    if(strcmp(fth->words[i].name, name) == 0)
      return i;
  return -1;
}

void forth_forgetWord(Forth *fth, char *name) {
  int d = forth_findWord(fth, name);
  if(d == -1) {
    printf(FORTH_UNDEFINED_ERR, name);
    return;
  }

  int pc = fth->words[d].addr;
  while(fth->program[pc] != FORTH_RET) {
    switch(fth->program[pc]) {
    case FORTH_PRINTSTRING:
    case FORTH_FORGET:
    case FORTH_VARIABLE:
    case FORTH_CREATE:
    case FORTH_CONSTANT:
    case FORTH_INCLUDE:
      free(forth_getValue(fth, pc+1));
      break;
    }

    forth_nextInstruction(fth, &pc);
  }

  int size = pc - fth->words[d].addr + 1;
  fth->size -= size;
  fth->old_size -= size;

  for(int i = fth->words[d].addr; i < fth->size; i++)
    fth->program[i] = fth->program[i+size];

  pc = 0;
  while(pc < fth->size) {
    intmax_t a;
    switch(fth->program[pc]) {
    case FORTH_JUMP:
    case FORTH_JZ:
    case FORTH_CALL:
    case FORTH_LOOP:
      a = (intmax_t)forth_getValue(fth, pc+1);
      if(a > fth->words[d].addr)
        forth_setValue(fth, pc+1, (void*)(a-size));
      break;
    }

    forth_nextInstruction(fth, &pc);
  }

  for(int i = d+1; i < fth->num_words; i++)
    fth->words[i].addr -= size;

  pc = 0;
  while(pc < fth->size) {
    if(fth->program[pc] == FORTH_CALL) {
      int i = forth_getValueIndex(fth, pc+1);
      if(i == d)
        forth_setValueIndex(fth, pc+1, fth->num_words-1);
      else if(i == fth->num_words-1)
        forth_setValueIndex(fth, pc+1, d);
    }

    forth_nextInstruction(fth, &pc);
  }

  free(fth->words[d].name);
  fth->num_words--;

  for(int i = d; i < fth->num_words; i++)
    fth->words[i] = fth->words[i+1];
}

void forth_create(Forth *fth, char *name, void *val) {
  int d = forth_findWord(fth, name);
  if(d != -1)
    forth_forgetWord(fth, name);

  const int offset = 4;

  for(int i = fth->size-1; i >= fth->old_size; i--)
    fth->program[i] = fth->program[i-offset];

  int size = fth->size;
  fth->size = fth->old_size;

  forth_addWord(fth, name);
  forth_addInstruction(fth, FORTH_PUSH);
  forth_addValue(fth, val);
  forth_addInstruction(fth, FORTH_RET);

  fth->size = size;

  int pc = fth->old_size;
  while(pc < fth->size) {
    switch(fth->program[pc]) {
    case FORTH_CALL:
    case FORTH_LOOP:
    case FORTH_JZ:
    case FORTH_JUMP:
      forth_setValue(fth, pc+1, forth_getValue(fth, pc+1)+offset);
      break;
    }
    forth_nextInstruction(fth, &pc);
  }

  fth->old_size += offset;
}

void forth_addFunction(Forth *fth, void (*fun)(Forth*), const char *name) {
  if(!forth_done(fth)) {
    printf(FORTH_FUNCTION_ERR);
    return;
  }

  char *s = malloc(strlen(name)+1);
  strcpy(s, name);
  forth_capitalize(s);

  if(!forth_validIdentifier(s)) {
    printf(FORTH_IDENTIFIER_ERR, name);
    free(s);
    return;
  }

  if(forth_findWord(fth, s) != -1)
    forth_forgetWord(fth, s);

  forth_addWord(fth, s);
  forth_addInstruction(fth, FORTH_FUNCTION);
  forth_addValue(fth, (void*)fun);
  forth_addInstruction(fth, FORTH_RET);

  fth->old_size = fth->size;

  free(s);
}
