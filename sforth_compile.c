#include "sforth.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

void forth_capitalize(char *s) {
  for(char *c = s; *c; c++)
    if(*c >= 'a' && *c <= 'z')
      *c += 'A'-'a';
}

void forth_compileToken(Forth *fth, char *s) {
  if(fth->quit)
    return;

  intmax_t d;

  /*printf("%s\n", s);*/

  switch(fth->mode) {
  case FORTHMODE_QUOTE:
    fth->mode = fth->old_mode;
    forth_addInstruction(fth, FORTH_PRINTSTRING);
    forth_addString(fth, s);
    return;

  case FORTHMODE_VARIABLE:
    forth_capitalize(s);
    fth->mode = fth->old_mode;
    if(!forth_validIdentifier(s))
      printf(FORTH_IDENTIFIER_ERR, s);
    else {
      forth_addInstruction(fth, FORTH_VARIABLE);
      forth_addString(fth, s);
    }
    break;

  case FORTHMODE_CONSTANT:
    forth_capitalize(s);
    fth->mode = fth->old_mode;
    if(!forth_validIdentifier(s))
      printf(FORTH_IDENTIFIER_ERR, s);
    else {
      forth_addInstruction(fth, FORTH_CONSTANT);
      forth_addString(fth, s);
    }
    break;

  case FORTHMODE_FORGET:
    forth_capitalize(s);
    fth->mode = fth->old_mode;
    if(!forth_validIdentifier(s))
      printf(FORTH_IDENTIFIER_ERR, s);
    else {
      forth_addInstruction(fth, FORTH_FORGET);
      forth_addString(fth, s);
    }
    break;

  case FORTHMODE_CREATE:
    forth_capitalize(s);
    fth->mode = fth->old_mode;
    if(!forth_validIdentifier(s))
      printf(FORTH_IDENTIFIER_ERR, s);
    else {
      forth_addInstruction(fth, FORTH_CREATE);
      forth_addString(fth, s);
    }
    break;

  case FORTHMODE_WORDNAME:
    forth_capitalize(s);
    if(!forth_validIdentifier(s)) {
      printf(FORTH_IDENTIFIER_ERR, s);
      fth->mode = FORTHMODE_WORDERR;
      return;
    }
    forth_addWord(fth, s);
    fth->mode = FORTHMODE_WORD;
    return;

  case FORTHMODE_WORDERR:
    if(strcmp(s, ";") == 0) {
      free(fth->words[--(fth->num_words)].name);
      fth->mode = FORTHMODE_NORMAL;
      fth->size = fth->old_size;
    }
    return;

  case FORTHMODE_WORD:
    if(strcmp(s, ";") == 0) {
      if(fth->if_sp || fth->do_sp || fth->begin_sp) {
        if(fth->if_sp)
          printf(FORTH_IF_ERR);
        if(fth->do_sp)
          printf(FORTH_DO_ERR);
        if(fth->begin_sp)
          printf(FORTH_BEGIN_ERR);
        fth->if_sp = 0;
        fth->do_sp = 0;
        fth->begin_sp = 0;
        free(fth->words[--(fth->num_words)].name);
        fth->size = fth->old_size;
        return;
      }

      forth_addInstruction(fth, FORTH_RET);
      fth->mode = FORTHMODE_NORMAL;
      d = forth_findWord(fth, fth->words[fth->num_words-1].name);
      if(d != -1 && d != fth->num_words-1)
        forth_forgetWord(fth, fth->words[fth->num_words-1].name);
      fth->old_size = fth->size;
      /*forth_printProgram(fth);*/
      return;
    }
    if(strcmp(s, ":") == 0) {
      printf(FORTH_COLON_ERR);
      fth->mode = FORTHMODE_WORDERR;
      return;
    }
    if(strcmp(s, "RECURSE") == 0) {
      forth_addInstruction(fth, FORTH_CALL);
      forth_addValue(fth, (void*)(intmax_t)fth->words[fth->num_words-1].addr);
      return;
    }

  case FORTHMODE_NORMAL:
    forth_capitalize(s);
    if(forth_isInteger(s, &d)) {
      forth_addInstruction(fth, FORTH_PUSH);
      forth_addValue(fth, (void*)d);
    }
    else if(strcmp(s, ".\"") == 0) {
      fth->old_mode = fth->mode;
      fth->mode = FORTHMODE_QUOTE;
    }
    else if(strcmp(s, ":") == 0)
      fth->mode = FORTHMODE_WORDNAME;
    else if(strcmp(s, "IF") == 0) {
      forth_addInstruction(fth, FORTH_JZ);
      forth_addValue(fth, 0);
      fth->if_a[fth->if_sp] = fth->size;
      fth->else_a[fth->if_sp++] = -1;
    }
    else if(strcmp(s, "ELSE") == 0) {
      if(!fth->if_sp)
        printf(FORTH_ELSE_ERR);
      else {
        forth_addInstruction(fth, FORTH_JUMP);
        forth_addValue(fth, 0);
        fth->else_a[fth->if_sp-1] = fth->size;
      }
    }
    else if(strcmp(s, "THEN") == 0) {
      if(!fth->if_sp)
        printf(FORTH_THEN_ERR);
      else {
        fth->if_sp--;
        if(fth->else_a[fth->if_sp] == -1)
          forth_setValue(fth, fth->if_a[fth->if_sp]-2,
              (void*)(intmax_t)fth->size);
        else {
          forth_setValue(fth, fth->if_a[fth->if_sp]-2,
              (void*)(intmax_t)fth->else_a[fth->if_sp]);
          forth_setValue(fth, fth->else_a[fth->if_sp]-2,
              (void*)(intmax_t)fth->size);
        }
      }
    }
    else if(strcmp(s, "DO") == 0) {
      forth_addInstruction(fth, FORTH_DO);
      fth->do_a[fth->do_sp++] = fth->size;
    }
    else if(strcmp(s, "I") == 0) {
      if(!fth->do_sp)
        printf(FORTH_I_ERR);
      else
        forth_addInstruction(fth, FORTH_I);
    }
    else if(strcmp(s, "LOOP") == 0) {
      if(!fth->do_sp)
        printf(FORTH_LOOP_ERR);
      else {
        forth_addInstruction(fth, FORTH_LOOP);
        forth_addValue(fth, (void*)(intmax_t)fth->do_a[--(fth->do_sp)]);
      }
    }
    else if(strcmp(s, "BEGIN") == 0)
      fth->begin_a[fth->begin_sp++] = fth->size;
    else if(strcmp(s, "UNTIL") == 0) {
      if(!fth->begin_sp)
        printf(FORTH_UNTIL_ERR);
      else {
        forth_addInstruction(fth, FORTH_JZ);
        forth_addValue(fth, (void*)(intmax_t)fth->begin_a[--(fth->begin_sp)]);
      }
    }
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
    else if(strcmp(s, "PRINTPROGRAM") == 0)
      forth_addInstruction(fth, FORTH_PRINTPROGRAM);
    else if((d = forth_findWord(fth, s)) != -1) {
      forth_addInstruction(fth, FORTH_CALL);
      forth_addValue(fth, (void*)(intmax_t)fth->words[d].addr);
    }
    else
      printf(FORTH_UNDEFINED_ERR, s);
    break;
  }

  if(fth->mode == FORTHMODE_NORMAL && fth->size != fth->old_size) {
    if(fth->begin_sp)
      return;
    if(fth->if_sp)
      return;
    if(fth->do_sp)
      return;

    forth_run(fth, fth->old_size);
    fth->size = fth->old_size;
  }
}
