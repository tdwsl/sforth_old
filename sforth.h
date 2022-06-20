#ifndef SFORTH_H
#define SFORTH_H

#include <stdint.h>

#define FORTH_STACK_SIZE 1024
#define FORTH_COMPILE_STACK_SIZE 256
#define FORTH_MEMORY_SIZE 480*1024

typedef struct forthWord {
  char *name;
  char *program;
  int size;
  void **values;
  int num_values;
} ForthWord;

typedef struct forth {
  ForthWord *words;
  int num_words;
  int fence;

  int if_a[FORTH_COMPILE_STACK_SIZE];
  int else_a[FORTH_COMPILE_STACK_SIZE];
  int if_sp;

  int begin_a[FORTH_COMPILE_STACK_SIZE];
  int while_a[FORTH_COMPILE_STACK_SIZE];
  int begin_sp;

  int do_a[FORTH_COMPILE_STACK_SIZE];
  int do_sp;

  char memory[FORTH_MEMORY_SIZE];
  void *here;

  void *stack[FORTH_STACK_SIZE];
  int sp;

  char mode, old_mode;

  int trace;

  char *name;
  char **names;
  int num_names, name_i;

  void (*emit)(char);
  char (*key)(void);

  int quit;
} Forth;

Forth *forth_newForth();
void forth_freeForth(Forth *fth);

int forth_has(Forth *fth, int n);
intmax_t forth_pop(Forth *fth);
void forth_push(Forth *fth, intmax_t val);

int forth_done(Forth *fth);

void forth_printStack(Forth *fth);

void forth_addFunction(Forth *fth, void (*fun)(Forth*), const char *name);
void forth_create(Forth *fth, char *name, void *val);

void forth_doString(Forth *fth, const char *text);
void forth_doFile(Forth *fth, const char *filename);

#endif
