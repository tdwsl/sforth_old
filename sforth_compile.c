#include "sforth.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

void forth_capitalize(char *s) {
  for(char *c = s; *c; c++)
    if(*c >= 'a' && *c <= 'z')
      *c += 'A'-'a';
}

void forth_compileToken(Forth *fth, char *s) {
  if(fth->quit)
    return;

  int d;

  /*forth_printf(fth, "%s\n", s);*/

  switch(fth->mode) {
  case FORTHMODE_QUOTE:
    fth->mode = fth->old_mode;
    forth_addInstruction(fth, FORTH_PRINTSTRING);
    forth_addString(fth, s);
    return;

  case FORTHMODE_VARIABLE:
    forth_capitalize(s);
    fth->mode = fth->old_mode;
    forth_addInstruction(fth, FORTH_VARIABLE);
    forth_addString(fth, s);
    break;

  case FORTHMODE_CONSTANT:
    forth_capitalize(s);
    fth->mode = fth->old_mode;
    forth_addInstruction(fth, FORTH_CONSTANT);
    forth_addString(fth, s);
    break;

  case FORTHMODE_FORGET:
    forth_capitalize(s);
    fth->mode = fth->old_mode;
    forth_addInstruction(fth, FORTH_FORGET);
    forth_addString(fth, s);
    break;

  case FORTHMODE_CREATE:
    forth_capitalize(s);
    fth->mode = fth->old_mode;
    forth_addInstruction(fth, FORTH_CREATE);
    forth_addString(fth, s);
    break;

  case FORTHMODE_WORDNAME:
    forth_capitalize(s);

    d = forth_findWord(fth, s);
    if(d != -1)
      forth_forgetWord(fth, s);
    forth_addWord(fth, s);
    fth->mode = FORTHMODE_WORD;
    return;

  case FORTHMODE_WORDERR:
    if(strcmp(s, ";") == 0) {
      fth->mode = FORTHMODE_NORMAL;
      fth->size = fth->old_size;
    }
    return;

  case FORTHMODE_WORD:
    if(strcmp(s, ";") == 0) {
      forth_addInstruction(fth, FORTH_RET);
      fth->old_size = fth->size;
      fth->mode = FORTHMODE_NORMAL;
      /*forth_printProgram(fth);*/
      return;
    }
    if(strcmp(s, ":") == 0) {
      forth_printf(fth, FORTH_COLON_ERR);
      fth->mode = FORTHMODE_WORDERR;
      free(fth->words[--(fth->num_words)].name);
      return;
    }
    if(strcmp(s, "RECURSE") == 0) {
      forth_addInstruction(fth, FORTH_CALL);
      forth_addValue(fth, (void*)(intmax_t)fth->words[fth->num_words-1].addr);
      return;
    }

  case FORTHMODE_NORMAL:
    forth_capitalize(s);
    if(forth_isInteger(s, &d))
      forth_push(fth, (void*)(intmax_t)d);
    else if(strcmp(s, ".\"") == 0) {
      fth->old_mode = fth->mode;
      fth->mode = FORTHMODE_QUOTE;
    }
    else if(strcmp(s, ":") == 0)
      fth->mode = FORTHMODE_WORDNAME;
    else if(strcmp(s, "FORGET") == 0) {
      fth->old_mode = fth->mode;
      fth->mode = FORTHMODE_FORGET;
    }
    else if(strcmp(s, "CREATE") == 0) {
      fth->old_mode = fth->mode;
      fth->mode = FORTHMODE_CREATE;
    }
    else if(strcmp(s, "VARIABLE") == 0) {
      fth->old_mode = fth->mode;
      fth->mode = FORTHMODE_VARIABLE;
    }
    else if(strcmp(s, "CONSTANT") == 0) {
      fth->old_mode = fth->mode;
      fth->mode = FORTHMODE_CONSTANT;
    }
    else if((d = forth_findWord(fth, s)) != -1) {
      forth_addInstruction(fth, FORTH_CALL);
      forth_addValue(fth, (void*)(intmax_t)fth->words[d].addr);
    }
    else
      forth_printf(fth, FORTH_UNDEFINED_ERR, s);
    break;
  }

  if(fth->mode == FORTHMODE_NORMAL && fth->size != fth->old_size) {
    if(fth->compile.begin_sp)
      return;
    if(fth->compile.if_sp)
      return;
    if(fth->compile.do_sp)
      return;

    forth_run(fth, fth->old_size);
    fth->size = fth->old_size;
  }
}
