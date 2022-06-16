#include "sforth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FORTH_UNDERFLOW_ERR "stack underflow !\n"
#define FORTH_UNDEFINED_ERR "%s ?\n"
#define FORTH_COLON_ERR "cannot define word inside word !\n"
#define FORTH_FILENOTFOUND_ERR "failed to open %s !\n"
#define FORTH_IF_ERR "expect 'THEN' after 'IF' !\n"
#define FORTH_ELSE_ERR "expect 'IF' before 'ELSE' !\n"
#define FORTH_THEN_ERR "expect 'IF' before 'THEN' !\n"
#define FORTH_DO_ERR "expect 'LOOP' after 'DO' !\n"
#define FORTH_I_ERR "expect 'DO' before 'I' !\n"
#define FORTH_LOOP_ERR "expect 'DO' before 'LOOP' !\n"
#define FORTH_PLUSLOOP_ERR "expect 'DO' before '+LOOP' !\n"
#define FORTH_BEGIN_ERR "expect 'UNTIL' after 'BEGIN' !\n"
#define FORTH_UNTIL_ERR "expect 'BEGIN' before 'UNTIL' !\n"
#define FORTH_WHILE_ERR "expect 'BEGIN' before 'WHILE' !\n"
#define FORTH_WHILEUNTIL_ERR "expect 'REPEAT' after 'WHILE' !\n"
#define FORTH_REPEATBEGIN_ERR "expect 'BEGIN' before 'REPEAT' !\n"
#define FORTH_REPEATWHILE_ERR "expect 'WHILE' before 'REPEAT' !\n"
#define FORTH_REPEAT_ERR "expect 'REPEAT' after 'WHILE' !\n"
#define FORTH_FUNCTION_ERR "cannot define function here !\n"
#define FORTH_IDENTIFIER_ERR "invalid identifier '%s' !\n"
#define FORTH_RESERVED_ERR "cannot redefine '%s' !\n"

/* bytecode instructions */
enum {
  FORTH_PUSH,
  FORTH_DROP,
  FORTH_DUP,
  FORTH_OVER,
  FORTH_SWAP,
  FORTH_ROT,
  FORTH_CALL,
  FORTH_JUMP,
  FORTH_JZ,
  FORTH_LOOP,
  FORTH_PLUSLOOP,
  FORTH_DIV,
  FORTH_MOD,
  FORTH_MUL,
  FORTH_PLUS,
  FORTH_MINUS,
  FORTH_PRINTSTRING,
  FORTH_CR,
  FORTH_PRINT,
  FORTH_DEC,
  FORTH_INC,
  FORTH_GREATER,
  FORTH_LESS,
  FORTH_EQUAL,
  FORTH_SETMEM,
  FORTH_GETMEM,
  FORTH_DO,
  FORTH_HERE,
  FORTH_ALLOT,
  FORTH_CREATE,
  FORTH_VARIABLE,
  FORTH_CONSTANT,
  FORTH_FORGET,
  FORTH_BYE,
  FORTH_SEE,
  FORTH_I,
  FORTH_AND,
  FORTH_OR,
  FORTH_XOR,
  FORTH_INVERT,
  FORTH_EQUALZ,
  FORTH_FUNCTION,
  FORTH_TRACEON,
  FORTH_TRACEOFF,
  FORTH_EMIT,
  FORTH_KEY,
  FORTH_2DUP,
  FORTH_2OVER,
  FORTH_GREATEREQ,
  FORTH_LESSEQ,
  FORTH_DIVMOD,
  FORTH_CELLS,
  FORTH_CELLPLUS,
  FORTH_NEQUALZ,
  FORTH_GREATERZ,
  FORTH_LESSZ,
  FORTH_NEQUAL,
  FORTH_VALUE,
  FORTH_TO,
  FORTH_DEPTH,
};

/* interpreter modes */
enum {
  FORTHMODE_NORMAL,
  FORTHMODE_QUOTE,
  FORTHMODE_WORDNAME,
  FORTHMODE_WORD,
  FORTHMODE_WORDERR,
  FORTHMODE_CREATE,
  FORTHMODE_VARIABLE,
  FORTHMODE_CONSTANT,
  FORTHMODE_INCLUDE,
  FORTHMODE_FORGET,
  FORTHMODE_SEE,
  FORTHMODE_VALUE,
  FORTHMODE_TO,
};

/* reserved words */
const char *forth_reserved[] = {
  ":",";","\\","(",".\"",".(",".'",
  "TO","IF","DO","SEE","THEN","ELSE","LOOP","+LOOP",
  "BEGIN","UNTIL","WHILE","VALUE","CREATE","FORGET",
  "INCLUDE","VARIABLE","CONSTANT","REPEAT","RECURSE",
  0,
};

void forth_addWord(Forth *fth, char *name) {
  fth->words = (ForthWord*)realloc(fth->words,
      sizeof(ForthWord)*(++(fth->num_words)));
  ForthWord *wd = &fth->words[fth->num_words-1];
  *wd = (ForthWord){0, 0, 0, 0, 0};
  wd->name = (char*)malloc(strlen(name)+1);
  strcpy(wd->name, name);
}

void forth_addInstruction(Forth *fth, char ins) {
  ForthWord *wd = &fth->words[fth->num_words-1];
  wd->program = (char*)realloc(wd->program, ++(wd->size));
  wd->program[wd->size-1] = ins;
}

void forth_addValue(Forth *fth, void *val) {
  ForthWord *wd = &fth->words[fth->num_words-1];
  wd->size += 2;
  wd->program = (char*)realloc(wd->program, wd->size);
  wd->program[wd->size-2] = (char)((wd->num_values)>>8);
  wd->program[wd->size-1] = (char)((wd->num_values)&0xff);
  wd->values = (void*)realloc(wd->values, sizeof(void*)*(++(wd->num_values)));
  wd->values[wd->num_values-1] = val;
}

void forth_addString(Forth *fth, char *s) {
  char *v = (char*)malloc(strlen(s)+1);
  strcpy(v, s);
  forth_addValue(fth, (void*)v);
}

int forth_getValueIndex(ForthWord *wd, int addr) {
  int i = ((int)(wd->program[addr]))<<8 | (int)(wd->program[addr+1]);
  return i;
}

void *forth_getValue(ForthWord *wd, int addr) {
  return wd->values[forth_getValueIndex(wd, addr)];
}

void forth_setValue(ForthWord *wd, int addr, void *val) {
  wd->values[forth_getValueIndex(wd, addr)] = val;
}

void forth_nextInstruction(ForthWord *wd, int *pc) {
  switch(wd->program[*pc]) {
  case FORTH_PRINTSTRING:
  case FORTH_CREATE:
  case FORTH_VARIABLE:
  case FORTH_CONSTANT:
  case FORTH_FORGET:
  case FORTH_SEE:
  case FORTH_JUMP:
  case FORTH_JZ:
  case FORTH_CALL:
  case FORTH_LOOP:
  case FORTH_PLUSLOOP:
  case FORTH_PUSH:
  case FORTH_FUNCTION:
  case FORTH_TO:
  case FORTH_VALUE:
    *pc += 3;
    break;
  default:
    (*pc)++;
    break;
  }
}

void forth_freeWord(ForthWord *wd) {
  int pc = 0;
  while(pc < wd->size) {
    switch(wd->program[pc]) {
    case FORTH_PRINTSTRING:
    case FORTH_CREATE:
    case FORTH_VARIABLE:
    case FORTH_CONSTANT:
    case FORTH_FORGET:
    case FORTH_SEE:
    case FORTH_VALUE:
    case FORTH_TO:
      free(forth_getValue(wd, pc+1));
      break;
    }
    forth_nextInstruction(wd, &pc);
  }
  free(wd->values);
  free(wd->program);
  free(wd->name);
}

void forth_copyWord(Forth *fth, ForthWord *wd) {
  int pc = 0;
  while(pc < 0) {
    switch(wd->program[pc]) {
    case FORTH_PRINTSTRING:
    case FORTH_CREATE:
    case FORTH_VARIABLE:
    case FORTH_CONSTANT:
    case FORTH_FORGET:
    case FORTH_SEE:
    case FORTH_TO:
    case FORTH_VALUE:
      forth_addString(fth, (char*)forth_getValue(wd, pc+1));
      break;
    case FORTH_JUMP:
    case FORTH_JZ:
    case FORTH_CALL:
    case FORTH_LOOP:
    case FORTH_PLUSLOOP:
    case FORTH_PUSH:
      forth_addValue(fth, forth_getValue(wd, pc+1));
      break;
    }
    forth_nextInstruction(wd, &pc);
  }
}

char forth_defaultKey() {
  char c = fgetc(stdin);
  while(fgetc(stdin) != '\n');
  return c;
}

void forth_defaultEmit(char c) {
  printf("%c", c);
}

Forth *forth_newForth() {
  Forth *fth = (Forth*)malloc(sizeof(Forth));

  fth->if_sp = 0;
  fth->do_sp = 0;
  fth->begin_sp = 0;

  fth->trace = 0;
  fth->quit = 0;

  fth->mode = FORTHMODE_NORMAL;

  fth->num_words = 0;
  fth->words = 0;

  fth->here = fth->memory;

  /* default key and emit functions */

  fth->emit = forth_defaultEmit; /* also called for printing strings */
  fth->key = forth_defaultKey;

  /* add default words, raise fence */

  forth_addWord(fth, "+");
  forth_addInstruction(fth, FORTH_PLUS);
  forth_addWord(fth, "-");
  forth_addInstruction(fth, FORTH_MINUS);
  forth_addWord(fth, "*");
  forth_addInstruction(fth, FORTH_MUL);
  forth_addWord(fth, "/");
  forth_addInstruction(fth, FORTH_DIV);
  forth_addWord(fth, "MOD");
  forth_addInstruction(fth, FORTH_MOD);
  forth_addWord(fth, "/MOD");
  forth_addInstruction(fth, FORTH_DIVMOD);
  forth_addWord(fth, "DROP");
  forth_addInstruction(fth, FORTH_DROP);
  forth_addWord(fth, "DUP");
  forth_addInstruction(fth, FORTH_DUP);
  forth_addWord(fth, "2DUP");
  forth_addInstruction(fth, FORTH_2DUP);
  forth_addWord(fth, "2OVER");
  forth_addInstruction(fth, FORTH_2OVER);
  forth_addWord(fth, "OVER");
  forth_addInstruction(fth, FORTH_OVER);
  forth_addWord(fth, "SWAP");
  forth_addInstruction(fth, FORTH_SWAP);
  forth_addWord(fth, "ROT");
  forth_addInstruction(fth, FORTH_ROT);
  forth_addWord(fth, "DEPTH");
  forth_addInstruction(fth, FORTH_DEPTH);
  forth_addWord(fth, "1-");
  forth_addInstruction(fth, FORTH_DEC);
  forth_addWord(fth, "1+");
  forth_addInstruction(fth, FORTH_INC);
  forth_addWord(fth, ">");
  forth_addInstruction(fth, FORTH_GREATER);
  forth_addWord(fth, "<");
  forth_addInstruction(fth, FORTH_LESS);
  forth_addWord(fth, "=");
  forth_addInstruction(fth, FORTH_EQUAL);
  forth_addWord(fth, "<>");
  forth_addInstruction(fth, FORTH_NEQUAL);
  forth_addWord(fth, ">=");
  forth_addInstruction(fth, FORTH_GREATEREQ);
  forth_addWord(fth, "<=");
  forth_addInstruction(fth, FORTH_LESSEQ);
  forth_addWord(fth, "0=");
  forth_addInstruction(fth, FORTH_EQUALZ);
  forth_addWord(fth, "0>");
  forth_addInstruction(fth, FORTH_GREATERZ);
  forth_addWord(fth, "0<");
  forth_addInstruction(fth, FORTH_LESSZ);
  forth_addWord(fth, "0<>");
  forth_addInstruction(fth, FORTH_NEQUALZ);
  forth_addWord(fth, "AND");
  forth_addInstruction(fth, FORTH_AND);
  forth_addWord(fth, "OR");
  forth_addInstruction(fth, FORTH_OR);
  forth_addWord(fth, "XOR");
  forth_addInstruction(fth, FORTH_XOR);
  forth_addWord(fth, "INVERT");
  forth_addInstruction(fth, FORTH_INVERT);
  forth_addWord(fth, "HERE");
  forth_addInstruction(fth, FORTH_HERE);
  forth_addWord(fth, "ALLOT");
  forth_addInstruction(fth, FORTH_ALLOT);
  forth_addWord(fth, "CELLS");
  forth_addInstruction(fth, FORTH_CELLS);
  forth_addWord(fth, "CELL+");
  forth_addInstruction(fth, FORTH_CELLPLUS);
  forth_addWord(fth, "!");
  forth_addInstruction(fth, FORTH_SETMEM);
  forth_addWord(fth, "@");
  forth_addInstruction(fth, FORTH_GETMEM);
  forth_addWord(fth, "KEY");
  forth_addInstruction(fth, FORTH_KEY);
  forth_addWord(fth, "EMIT");
  forth_addInstruction(fth, FORTH_EMIT);
  forth_addWord(fth, "BYE");
  forth_addInstruction(fth, FORTH_BYE);
  forth_addWord(fth, ".");
  forth_addInstruction(fth, FORTH_PRINT);
  forth_addWord(fth, "CR");
  forth_addInstruction(fth, FORTH_CR);
  forth_addWord(fth, "TRACEON");
  forth_addInstruction(fth, FORTH_TRACEON);
  forth_addWord(fth, "TRACEOFF");
  forth_addInstruction(fth, FORTH_TRACEOFF);

  fth->fence = fth->num_words;

  /* done */

  return fth;
}

void forth_freeForth(Forth *fth) {
  for(int i = 0; i < fth->num_words; i++)
    forth_freeWord(&fth->words[i]);

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

intmax_t forth_pop(Forth *fth) {
  if(forth_has(fth, 1))
    return (intmax_t)fth->stack[--(fth->sp)];
  else
    return 0;
}

void forth_push(Forth *fth, intmax_t val) {
  fth->stack[fth->sp++] = (void*)val;
}

int forth_wordIndex(Forth *fth, char *name) {
  for(int i = 0; i < fth->num_words; i++)
    if(strcmp(fth->words[i].name, name) == 0)
      return i;
  return -1;
}

void forth_forgetWord(Forth *fth, char *name) {
  int d = forth_wordIndex(fth, name);
  if(d == -1)
    return;

  forth_freeWord(&fth->words[d]);
  fth->num_words--;
  if(d < fth->fence)
    fth->fence--;

  for(int i = d; i < fth->num_words; i++)
    fth->words[i] = fth->words[i+1];

  for(int i = fth->fence; i < fth->num_words; i++) {
    int pc = 0;
    while(pc < fth->words[i].size) {
      if(fth->words[i].program[pc] == FORTH_CALL) {
        int j = (intmax_t)forth_getValue(&fth->words[i], pc+1);
        if(j > d)
          forth_setValue(&fth->words[i], pc+1, (void*)(intmax_t)(j-1));
        else if(j == d)
          forth_setValue(&fth->words[i], pc+1,
              (void*)(intmax_t)fth->num_words);
      }
      forth_nextInstruction(&fth->words[i], &pc);
    }
  }
}

void forth_forgetName(Forth *fth, char *name) {
  int d = forth_wordIndex(fth, name);
  if(d == -1)
    return;

  free(fth->words[d].name);
  fth->words[d].name = (char*)malloc(1);
  fth->words[d].name[0] = 0;
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

int forth_validIdentifier(char *name) {
  for(int i = 0; forth_reserved[i]; i++)
    if(strcmp(name, forth_reserved[i]) == 0) {
      printf(FORTH_RESERVED_ERR, name);
      return 0;
    }

  intmax_t n;
  if(forth_isInteger(name, &n) || !name[0]) {
    printf(FORTH_IDENTIFIER_ERR, name);
    return 0;
  }

  for(int i = 0; name[i]; i++)
    if(name[i] <= ' ') {
      printf(FORTH_IDENTIFIER_ERR, name);
      return 0;
    }

  return 1;
}

void forth_capitalize(char *s) {
  for(char *c = s; *c; c++)
    if(*c >= 'a' && *c <= 'z')
      *c += 'A'-'a';
}

void forth_create(Forth *fth, char *name, void *val) {
  if(!forth_validIdentifier(name))
    return;

  forth_forgetWord(fth, name);
  forth_addWord(fth, name);
  forth_addInstruction(fth, FORTH_PUSH);
  forth_addValue(fth, val);
}

void forth_addFunction(Forth *fth, void (*fun)(Forth*), const char *name) {
  if(!forth_validIdentifier((char*)name))
    return;

  char *s = (char*)malloc(strlen(name)+1);
  strcpy(s, name);
  forth_capitalize(s);
  forth_forgetWord(fth, s);
  forth_addWord(fth, s);
  free(s);

  forth_addInstruction(fth, FORTH_FUNCTION);
  forth_addValue(fth, (void*)fun);
}

void forth_printInstruction(Forth *fth, ForthWord *wd, int pc) {
  int i;
  printf("%d\t", pc);
  switch(wd->program[pc]) {
  case FORTH_PLUS: printf("+"); break;
  case FORTH_MINUS: printf("-"); break;
  case FORTH_MUL: printf("*"); break;
  case FORTH_DIV: printf("/"); break;
  case FORTH_MOD: printf("mod"); break;
  case FORTH_DUP: printf("dup"); break;
  case FORTH_ROT: printf("rot"); break;
  case FORTH_OVER: printf("over"); break;
  case FORTH_CR: printf("cr"); break;
  case FORTH_INC: printf("1+"); break;
  case FORTH_DEC: printf("1-"); break;
  case FORTH_GREATER: printf(">"); break;
  case FORTH_LESS: printf("<"); break;
  case FORTH_EQUAL: printf("="); break;
  case FORTH_NEQUAL: printf("<>"); break;
  case FORTH_DROP: printf("drop"); break;
  case FORTH_GETMEM: printf("@"); break;
  case FORTH_SETMEM: printf("!"); break;
  case FORTH_PRINT: printf("."); break;
  case FORTH_DO: printf("do"); break;
  case FORTH_HERE: printf("here"); break;
  case FORTH_ALLOT: printf("allot"); break;
  case FORTH_BYE: printf("bye"); break;
  case FORTH_I: printf("i"); break;
  case FORTH_AND: printf("and"); break;
  case FORTH_OR: printf("or"); break;
  case FORTH_XOR: printf("xor"); break;
  case FORTH_INVERT: printf("invert"); break;
  case FORTH_EQUALZ: printf("0="); break;
  case FORTH_SWAP: printf("swap"); break;
  case FORTH_TRACEON: printf("traceon"); break;
  case FORTH_TRACEOFF: printf("traceoff"); break;
  case FORTH_EMIT: printf("emit"); break;
  case FORTH_KEY: printf("key"); break;
  case FORTH_2DUP: printf("2dup"); break;
  case FORTH_2OVER: printf("2over"); break;
  case FORTH_DIVMOD: printf("/mod"); break;
  case FORTH_GREATEREQ: printf(">="); break;
  case FORTH_LESSEQ: printf("<="); break;
  case FORTH_CELLS: printf("cells"); break;
  case FORTH_CELLPLUS: printf("cells+"); break;
  case FORTH_GREATERZ: printf("0>"); break;
  case FORTH_LESSZ: printf("0<"); break;
  case FORTH_NEQUALZ: printf("0<>");
  case FORTH_DEPTH: printf("depth");

  case FORTH_PRINTSTRING:
    printf(".\"%s\"", (char*)forth_getValue(wd, pc+1)); break;
  case FORTH_PUSH:
    printf("push %jd", (intmax_t)forth_getValue(wd, pc+1)); break;
  case FORTH_CALL:
    i = (intmax_t)forth_getValue(wd, pc+1);
    if(i < fth->num_words)
      printf("call %s", fth->words[i].name);
    else
      printf("call %d", i);
    break;
  case FORTH_JUMP:
    printf("jump %jd", (intmax_t)forth_getValue(wd, pc+1)); break;
  case FORTH_JZ:
    printf("jz %jd", (intmax_t)forth_getValue(wd, pc+1)); break;
  case FORTH_LOOP:
    printf("loop %jd", (intmax_t)forth_getValue(wd, pc+1)); break;
  case FORTH_PLUSLOOP:
    printf("+loop %jd", (intmax_t)forth_getValue(wd, pc+1)); break;
  case FORTH_CREATE:
    printf("create %s", (char*)forth_getValue(wd, pc+1)); break;
  case FORTH_CONSTANT:
    printf("constant %s", (char*)forth_getValue(wd, pc+1)); break;
  case FORTH_VARIABLE:
    printf("variable %s", (char*)forth_getValue(wd, pc+1)); break;
  case FORTH_FORGET:
    printf("forget %s", (char*)forth_getValue(wd, pc+1)); break;
  case FORTH_VALUE:
    printf("value %s", (char*)forth_getValue(wd, pc+1)); break;
  case FORTH_TO:
    printf("to %s", (char*)forth_getValue(wd, pc+1)); break;
  case FORTH_SEE:
    printf("printdebug %s", (char*)forth_getValue(wd, pc+1)); break;
  case FORTH_FUNCTION:
    printf("function %jd", (intmax_t)forth_getValue(wd, pc+1)); break;
  }
  printf("\n");
}

void forth_printStack(Forth *fth) {
  printf("<");
  for(int i = 0; i < fth->sp; i++) {
    printf("%jd", (intmax_t)(fth->stack[i]));
    if(i < fth->sp-1)
      printf(" ");
  }
  printf(">\n");
}

void forth_printWord(Forth *fth, ForthWord *wd) {
  printf(": %s\n", wd->name);
  for(int pc = 0; pc < wd->size; forth_nextInstruction(wd, &pc))
    forth_printInstruction(fth, wd, pc);
}

void forth_runWord(Forth *fth, ForthWord *wd) {
  int pc = 0;

  int i_stack[FORTH_COMPILE_STACK_SIZE*2];
  int i_sp = 0;

  intmax_t v1, v2;
  int i;
  char *c;

  while(pc < wd->size && !(fth->quit)) {
    /* trace */
    if(fth->trace) {
      forth_printStack(fth);
      forth_printInstruction(fth, wd, pc);
    }

    switch(wd->program[pc]) {
    case FORTH_CALL:
      i = (intmax_t)forth_getValue(wd, pc+1);
      if(i < fth->num_words)
        forth_runWord(fth, &fth->words[i]);
      break;
    case FORTH_JUMP:
      pc = (intmax_t)forth_getValue(wd, pc+1);
      continue;
    case FORTH_JZ:
      if(!(intmax_t)forth_pop(fth)) {
        pc = (intmax_t)forth_getValue(wd, pc+1);
        continue;
      }
      break;
    case FORTH_PUSH:
      forth_push(fth, (intmax_t)forth_getValue(wd, pc+1));
      break;
    case FORTH_PRINTSTRING:
      for(c = (char*)forth_getValue(wd, pc+1); *c; c++)
        fth->emit(*c);
      break;
    case FORTH_PLUS:
      forth_push(fth, forth_pop(fth)+forth_pop(fth));
      break;
    case FORTH_MUL:
      forth_push(fth, forth_pop(fth)*forth_pop(fth));
      break;
    case FORTH_MOD:
      v1 = forth_pop(fth);
      forth_push(fth, forth_pop(fth)%v1);
      break;
    case FORTH_DIV:
      v1 = forth_pop(fth);
      forth_push(fth, forth_pop(fth)/v1);
      break;
    case FORTH_MINUS:
      v1 = forth_pop(fth);
      forth_push(fth, forth_pop(fth)-v1);
      break;
    case FORTH_CR:
      printf("\n");
      break;
    case FORTH_PRINT:
      printf("%jd ", forth_pop(fth));
      break;
    case FORTH_SETMEM:
      v2 = forth_pop(fth);
      v1 = forth_pop(fth);
      memcpy((void**)v2, (void*)&v1, sizeof(void*));
      break;
    case FORTH_GETMEM:
      forth_push(fth, *(intmax_t*)forth_pop(fth));
      break;
    case FORTH_DUP:
      v1 = forth_pop(fth);
      forth_push(fth, v1);
      forth_push(fth, v1);
      break;
    case FORTH_2DUP:
      if(forth_has(fth, 2)) {
        forth_push(fth, (intmax_t)fth->stack[fth->sp-2]);
        forth_push(fth, (intmax_t)fth->stack[fth->sp-2]);
      }
      else {
        forth_push(fth, 0);
        forth_push(fth, 0);
      }
      break;
    case FORTH_2OVER:
      if(forth_has(fth, 4)) {
        forth_push(fth, (intmax_t)fth->stack[fth->sp-4]);
        forth_push(fth, (intmax_t)fth->stack[fth->sp-4]);
      }
      else {
        forth_push(fth, 0);
        forth_push(fth, 0);
      }
      break;
    case FORTH_DEPTH:
      forth_push(fth, fth->sp);
      break;
    case FORTH_DIVMOD:
      if(forth_has(fth, 2)) {
        v2 = forth_pop(fth);
        v1 = forth_pop(fth);
        forth_push(fth, v1%v2);
        forth_push(fth, v1/v2);
      }
      else {
        forth_push(fth, 0);
        forth_push(fth, 0);
      }
    case FORTH_SWAP:
      if(forth_has(fth, 2)) {
        v1 = (intmax_t)fth->stack[fth->sp-1];
        fth->stack[fth->sp-1] = fth->stack[fth->sp-2];
        fth->stack[fth->sp-2] = (void*)v1;
      }
      break;
    case FORTH_OVER:
      if(forth_has(fth, 2))
        forth_push(fth, (intmax_t)fth->stack[fth->sp-2]);
      else
        forth_push(fth, 0);
      break;
    case FORTH_ROT:
      if(!forth_has(fth, 3))
        break;
      v1 = (intmax_t)fth->stack[fth->sp-3];
      fth->stack[fth->sp-3] = fth->stack[fth->sp-2];
      fth->stack[fth->sp-2] = fth->stack[fth->sp-1];
      fth->stack[fth->sp-1] = (void*)v1;
      break;
    case FORTH_DEC:
      if(forth_has(fth, 1))
        fth->stack[fth->sp-1]--;
      break;
    case FORTH_INC:
      if(forth_has(fth, 1))
        fth->stack[fth->sp-1]++;
      break;
    case FORTH_GREATER:
      v1 = forth_pop(fth);
      forth_push(fth, (forth_pop(fth) > v1)*-1);
      break;
    case FORTH_LESS:
      v1 = forth_pop(fth);
      forth_push(fth, (forth_pop(fth) < v1)*-1);
      break;
    case FORTH_GREATEREQ:
      v1 = forth_pop(fth);
      forth_push(fth, (forth_pop(fth) >= v1)*-1);
      break;
    case FORTH_LESSEQ:
      v1 = forth_pop(fth);
      forth_push(fth, (forth_pop(fth) <= v1)*-1);
      break;
    case FORTH_EQUAL:
      forth_push(fth, (forth_pop(fth) == forth_pop(fth))*-1);
      break;
    case FORTH_NEQUAL:
      forth_push(fth, (forth_pop(fth) != forth_pop(fth))*-1);
      break;
    case FORTH_EQUALZ:
      if(forth_has(fth, 1))
        forth_push(fth, (forth_pop(fth) == 0)*-1);
      break;
    case FORTH_NEQUALZ:
      if(forth_has(fth, 1))
        forth_push(fth, (forth_pop(fth) != 0)*-1);
      break;
    case FORTH_GREATERZ:
      if(forth_has(fth, 1))
        forth_push(fth, (forth_pop(fth) > 0)*-1);
      break;
    case FORTH_LESSZ:
      if(forth_has(fth, 1))
        forth_push(fth, (forth_pop(fth) < 0)*-1);
      break;
    case FORTH_DROP:
      forth_pop(fth);
      break;
    case FORTH_DO:
      i_sp += 2;
      i_stack[i_sp-1] = forth_pop(fth);
      i_stack[i_sp-2] = forth_pop(fth);
      break;
    case FORTH_LOOP:
      if(++i_stack[i_sp-1] >= i_stack[i_sp-2])
        i_sp -= 2;
      else {
        pc = (intmax_t)forth_getValue(wd, pc+1);
        continue;
      }
      break;
    case FORTH_PLUSLOOP:
      i = forth_pop(fth);
      i_stack[i_sp-1] += i;
      if(i_stack[i_sp-1] >= i_stack[i_sp-2]
          && i_stack[i_sp-1]-i < i_stack[i_sp-2])
        i_sp -= 2;
      else if(i_stack[i_sp-1] <= i_stack[i_sp-2]
          && i_stack[i_sp-1]-i > i_stack[i_sp-2])
        i_sp -= 2;
      else {
        pc = (intmax_t)forth_getValue(wd, pc+1);
        continue;
      }
      break;
    case FORTH_I:
      forth_push(fth, i_stack[i_sp-1]);
      break;
    case FORTH_HERE:
      forth_push(fth, (intmax_t)fth->here);
      break;
    case FORTH_ALLOT:
      fth->here += forth_pop(fth);
      break;
    case FORTH_CELLS:
      forth_push(fth, forth_pop(fth)*sizeof(void*));
      break;
    case FORTH_CELLPLUS:
      forth_push(fth, forth_pop(fth)+sizeof(void*));
      break;
    case FORTH_CREATE:
      forth_create(fth, (char*)forth_getValue(wd, pc+1), fth->here);
      break;
    case FORTH_VARIABLE:
      forth_create(fth, (char*)forth_getValue(wd, pc+1), fth->here);
      fth->here += sizeof(void*);
      break;
    case FORTH_CONSTANT:
      forth_create(fth, (char*)forth_getValue(wd, pc+1),
          (void*)forth_pop(fth));
      break;
    case FORTH_VALUE:
      forth_create(fth, (char*)forth_getValue(wd, pc+1), fth->here);
      forth_addInstruction(fth, FORTH_GETMEM);
      v1 = forth_pop(fth);
      memcpy((void**)fth->here, &v1, sizeof(void*));
      fth->here += sizeof(void*);
      break;
    case FORTH_TO:
      i = forth_wordIndex(fth, (char*)forth_getValue(wd, pc+1));
      if(i != -1) {
        v1 = (intmax_t)forth_getValue(&fth->words[i], 1);
        v2 = forth_pop(fth);
        memcpy((void**)v1, &v2, sizeof(void*));
      }
      else
        printf(FORTH_IDENTIFIER_ERR, (char*)forth_getValue(wd, pc+1));
      break;
    case FORTH_FORGET:
      forth_forgetWord(fth, (char*)forth_getValue(wd, pc+1));
      break;
    case FORTH_SEE:
      i = forth_wordIndex(fth, (char*)forth_getValue(wd, pc+1));
      if(i == -1)
        printf(FORTH_UNDEFINED_ERR, (char*)forth_getValue(wd, pc+1));
      else
        forth_printWord(fth, &fth->words[i]);
      break;
    case FORTH_BYE:
      fth->quit = 1;
      return;
    case FORTH_AND:
      forth_push(fth, forth_pop(fth)&forth_pop(fth));
      break;
    case FORTH_OR:
      forth_push(fth, forth_pop(fth)|forth_pop(fth));
      break;
    case FORTH_XOR:
      forth_push(fth, forth_pop(fth)^forth_pop(fth));
      break;
    case FORTH_INVERT:
      if(forth_has(fth, 1))
        forth_push(fth, ~forth_pop(fth));
      break;
    case FORTH_FUNCTION:
      ((void (*)(Forth*))forth_getValue(wd, pc+1))(fth);
      break;
    case FORTH_TRACEON:
      fth->trace = 1;
      break;
    case FORTH_TRACEOFF:
      fth->trace = 0;
      break;
    case FORTH_EMIT:
      fth->emit(forth_pop(fth));
      break;
    case FORTH_KEY:
      forth_push(fth, fth->key());
      break;
    }

    forth_nextInstruction(wd, &pc);
  }
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

void forth_compileToken(Forth *fth, char *s) {
  if(fth->quit)
    return;

  intmax_t d;

  switch(fth->mode) {
  case FORTHMODE_QUOTE:
    fth->mode = fth->old_mode;
    forth_addInstruction(fth, FORTH_PRINTSTRING);
    forth_addString(fth, s);
    return;

  case FORTHMODE_INCLUDE:
    fth->mode = FORTHMODE_NORMAL;
    forth_doFile(fth, (const char*)s);
    break;

  case FORTHMODE_TO:
    forth_capitalize(s);
    fth->mode = fth->old_mode;
    if(forth_validIdentifier(s)) {
      forth_addInstruction(fth, FORTH_TO);
      forth_addString(fth, s);
    }
    break;

  case FORTHMODE_VALUE:
    forth_capitalize(s);
    fth->mode = fth->old_mode;
    if(forth_validIdentifier(s)) {
      forth_addInstruction(fth, FORTH_VALUE);
      forth_addString(fth, s);
    }
    break;

  case FORTHMODE_VARIABLE:
    forth_capitalize(s);
    fth->mode = fth->old_mode;
    if(forth_validIdentifier(s)) {
      forth_addInstruction(fth, FORTH_VARIABLE);
      forth_addString(fth, s);
    }
    break;

  case FORTHMODE_CONSTANT:
    forth_capitalize(s);
    fth->mode = fth->old_mode;
    if(forth_validIdentifier(s)) {
      forth_addInstruction(fth, FORTH_CONSTANT);
      forth_addString(fth, s);
    }
    break;

  case FORTHMODE_FORGET:
    forth_capitalize(s);
    fth->mode = fth->old_mode;
    if(forth_validIdentifier(s)) {
      forth_addInstruction(fth, FORTH_FORGET);
      forth_addString(fth, s);
    }
    break;

  case FORTHMODE_CREATE:
    forth_capitalize(s);
    fth->mode = fth->old_mode;
    if(forth_validIdentifier(s)) {
      forth_addInstruction(fth, FORTH_CREATE);
      forth_addString(fth, s);
    }
    break;

  case FORTHMODE_SEE:
    forth_capitalize(s);
    fth->mode = fth->old_mode;
    if(forth_validIdentifier(s)) {
      forth_addInstruction(fth, FORTH_SEE);
      forth_addString(fth, s);
    }
    break;

  case FORTHMODE_WORDNAME:
    forth_capitalize(s);
    if(!forth_validIdentifier(s)) {
      fth->mode = FORTHMODE_WORDERR;
      return;
    }
    forth_addWord(fth, s);
    fth->mode = FORTHMODE_WORD;
    return;

  case FORTHMODE_WORDERR:
    if(strcmp(s, ";") == 0) {
      forth_freeWord(&fth->words[--(fth->num_words)]);
      fth->mode = FORTHMODE_NORMAL;
    }
    return;

  case FORTHMODE_WORD:
    if(strcmp(s, ";") == 0) {
      if(fth->if_sp || fth->do_sp || fth->begin_sp) {
        if(fth->if_sp)
          printf(FORTH_IF_ERR);
        if(fth->do_sp)
          printf(FORTH_DO_ERR);
        if(fth->begin_sp) {
          if(fth->while_a[fth->begin_sp-1] == -1)
            printf(FORTH_BEGIN_ERR);
          else
            printf(FORTH_REPEAT_ERR);
        }
        fth->if_sp = 0;
        fth->do_sp = 0;
        fth->begin_sp = 0;
        forth_freeWord(&fth->words[--(fth->num_words)]);
        return;
      }

      fth->mode = FORTHMODE_NORMAL;
      d = forth_wordIndex(fth, fth->words[fth->num_words-1].name);
      if(d != -1 && d != fth->num_words-1)
        forth_forgetName(fth, fth->words[fth->num_words-1].name);
        /*forth_forgetWord(fth, fth->words[fth->num_words-1].name);*/
      return;
    }
    if(strcmp(s, ":") == 0) {
      printf(FORTH_COLON_ERR);
      fth->mode = FORTHMODE_WORDERR;
      return;
    }
    if(strcmp(s, "RECURSE") == 0) {
      forth_addInstruction(fth, FORTH_CALL);
      forth_addValue(fth, (void*)(intmax_t)(fth->num_words-1));
      return;
    }

  case FORTHMODE_NORMAL:
    if(forth_done(fth)) {
      d = forth_wordIndex(fth, "0");
      if(d == -1)
        forth_addWord(fth, "0");
      else if(d != fth->num_words-1) {
        forth_addWord(fth, "0");
        forth_copyWord(fth, &fth->words[d]);
        forth_forgetWord(fth, "0");
      }
    }

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
      fth->if_a[fth->if_sp] = fth->words[fth->num_words-1].size;
      fth->else_a[fth->if_sp++] = -1;
    }
    else if(strcmp(s, "ELSE") == 0) {
      if(!fth->if_sp)
        printf(FORTH_ELSE_ERR);
      else {
        forth_addInstruction(fth, FORTH_JUMP);
        forth_addValue(fth, 0);
        fth->else_a[fth->if_sp-1] = fth->words[fth->num_words-1].size;
      }
    }
    else if(strcmp(s, "THEN") == 0) {
      if(!fth->if_sp)
        printf(FORTH_THEN_ERR);
      else {
        fth->if_sp--;
        ForthWord *wd = &fth->words[fth->num_words-1];
        if(fth->else_a[fth->if_sp] == -1)
          forth_setValue(wd, fth->if_a[fth->if_sp]-2,
              (void*)(intmax_t)wd->size);
        else {
          forth_setValue(wd, fth->if_a[fth->if_sp]-2,
              (void*)(intmax_t)fth->else_a[fth->if_sp]);
          forth_setValue(wd, fth->else_a[fth->if_sp]-2,
              (void*)(intmax_t)wd->size);
        }
      }
    }
    else if(strcmp(s, "DO") == 0) {
      forth_addInstruction(fth, FORTH_DO);
      fth->do_a[fth->do_sp++] = fth->words[fth->num_words-1].size;
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
    else if(strcmp(s, "+LOOP") == 0) {
      if(!fth->do_sp)
        printf(FORTH_PLUSLOOP_ERR);
      else {
        forth_addInstruction(fth, FORTH_PLUSLOOP);
        forth_addValue(fth, (void*)(intmax_t)fth->do_a[--(fth->do_sp)]);
      }
    }
    else if(strcmp(s, "BEGIN") == 0) {
      fth->begin_a[fth->begin_sp] = fth->words[fth->num_words-1].size;
      fth->while_a[fth->begin_sp++] = -1;
    }
    else if(strcmp(s, "WHILE") == 0) {
      if(!fth->begin_sp)
        printf(FORTH_WHILE_ERR);
      else {
        forth_addInstruction(fth, FORTH_NEQUALZ);
        forth_addInstruction(fth, FORTH_JZ);
        forth_addValue(fth, 0);
        fth->while_a[fth->begin_sp-1] = fth->words[fth->num_words-1].size;
      }
    }
    else if(strcmp(s, "UNTIL") == 0) {
      if(!fth->begin_sp)
        printf(FORTH_UNTIL_ERR);
      else if(fth->while_a[fth->begin_sp-1] != -1)
        printf(FORTH_WHILEUNTIL_ERR);
      else {
        forth_addInstruction(fth, FORTH_JZ);
        forth_addValue(fth, (void*)(intmax_t)fth->begin_a[--(fth->begin_sp)]);
      }
    }
    else if(strcmp(s, "REPEAT") == 0) {
      if(!fth->begin_sp)
        printf(FORTH_REPEATBEGIN_ERR);
      else if(fth->while_a[fth->begin_sp-1] == -1)
        printf(FORTH_REPEATWHILE_ERR);
      else {
        ForthWord *wd = &fth->words[fth->num_words-1];
        forth_addInstruction(fth, FORTH_JUMP);
        forth_addValue(fth, (void*)(intmax_t)fth->begin_a[--(fth->begin_sp)]);
        forth_setValue(wd, fth->while_a[fth->begin_sp]-2,
            (void*)(intmax_t)wd->size);
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
    else if(strcmp(s, "VALUE") == 0) {
      fth->old_mode = fth->mode;
      fth->mode = FORTHMODE_VALUE;
    }
    else if(strcmp(s, "TO") == 0) {
      fth->old_mode = fth->mode;
      fth->mode = FORTHMODE_TO;
    }
    else if(strcmp(s, "VARIABLE") == 0) {
      fth->old_mode = fth->mode;
      fth->mode = FORTHMODE_VARIABLE;
    }
    else if(strcmp(s, "CONSTANT") == 0) {
      fth->old_mode = fth->mode;
      fth->mode = FORTHMODE_CONSTANT;
    }
    else if(strcmp(s, "SEE") == 0) {
      fth->old_mode = fth->mode;
      fth->mode = FORTHMODE_SEE;
    }
    else if(strcmp(s, "INCLUDE") == 0 && forth_done(fth))
      fth->mode = FORTHMODE_INCLUDE;
    else if((d = forth_wordIndex(fth, s)) != -1) {
      if(d < fth->fence) {
        for(int i = 0; i < fth->words[d].size; i++)
          forth_addInstruction(fth, fth->words[d].program[i]);
      }
      else {
        forth_addInstruction(fth, FORTH_CALL);
        forth_addValue(fth, (void*)(intmax_t)d);
      }
    }
    else
      printf(FORTH_UNDEFINED_ERR, s);
    break;
  }

  if(forth_done(fth)) {
    int d = forth_wordIndex(fth, "0");
    if(d != -1) {
      forth_runWord(fth, &fth->words[d]);
      forth_forgetWord(fth, "0");
    }
  }
}

void forth_doString(Forth *fth, const char *text) {
  int max = 100;
  char *s = (char*)malloc(max);
  const int buf = 40;
  int len = 0;

  char quote = 0;
  char comment = 0;

  for(const char *c = text; ; c++) {
    if(*c == '\n' || *c == 0) {
      if(quote || len) {
        s[len] = 0;
        if(strcmp(s, "\\") != 0)
          forth_compileToken(fth, s);
      }

      len = 0;
      quote = 0;
      comment = 0;

      if(*c == 0)
        break;
    }

    else if(comment) {
      if(comment == '(' && *c == ')')
        comment = 0;
      continue;
    }

    else if(quote) {
      if(*c == quote) {
        s[len] = 0;
        quote = 0;
        forth_compileToken(fth, s);
        len = 0;
      }
      else
        s[len++] = *c;
    }

    else if(*c == ' ' || *c == '\t') {
      if(len) {
        s[len] = 0;
        len = 0;

        if(strcmp(s, ".\"") == 0
            || strcmp(s, ".(") == 0
            || strcmp(s, ".'") == 0) {
          quote = s[1];
          s[1] = '"';
          if(quote == '(')
            quote = ')';
        }

        if(strcmp(s, "\\") == 0 || strcmp(s, "(") == 0)
          comment = s[0];
        else
          forth_compileToken(fth, s);
      }
    }

    else
      s[len++] = *c;

    if(len > max-buf) {
      max += buf;
      s = (char*)realloc(s, max);
    }
  }

  free(s);
}

void forth_doFile(Forth *fth, const char *filename) {
  FILE *fp = fopen(filename, "r");
  if(!fp) {
    printf(FORTH_FILENOTFOUND_ERR, filename);
    return;
  }

  int max = 100;
  char *s = (char*)malloc(max);
  int buf = 40;
  int len = 0;

  while(!feof(fp)) {
    char c = fgetc(fp);
    s[len++] = c;
    if(len > max-buf) {
      max += buf;
      s = (char*)realloc(s, max);
    }
  }
  fclose(fp);

  s[len-1] = 0;
  forth_doString(fth, s);

  free(s);
}
