#ifndef SFORTH_H
#define SFORTH_H

#define FORTH_STACK_SIZE 512
#define FORTH_COMPILE_STACK_SIZE 256
#define FORTH_MEMORY_SIZE 480*1024

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
#define FORTH_BEGIN_ERR "expect 'UNTIL' after 'BEGIN' !\n"
#define FORTH_UNTIL_ERR "expect 'BEGIN' before 'UNTIL' !\n"
#define FORTH_FUNCTION_ERR "cannot define function here !\n"
#define FORTH_IDENTIFIER_ERR "invalid identifier '%s' !\n"

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
  int begin_sp;

  int do_a[FORTH_COMPILE_STACK_SIZE];
  int do_sp;

  char memory[FORTH_MEMORY_SIZE];
  void *here;

  void *stack[FORTH_STACK_SIZE];
  int sp;

  char mode, old_mode;

  int trace;

  void (*emit)(char);
  char (*key)(void);

  int quit;
} Forth;

Forth *forth_newForth();
void forth_freeForth(Forth *fth);

int forth_has(Forth *fth, int n);
void *forth_pop(Forth *fth);
void forth_push(Forth *fth, void *val);

int forth_done(Forth *fth);

void forth_printStack(Forth *fth);

void forth_addFunction(Forth *fth, void (*fun)(Forth*), const char *name);
void forth_create(Forth *fth, char *name, void *val);

void forth_doString(Forth *fth, const char *text);
void forth_doFile(Forth *fth, const char *filename);

#endif
