#ifndef SFORTH_H
#define SFORTH_H

#include <stdarg.h>
#include <stdint.h>

#define FORTH_MEMORY_SIZE 64*1024
#define FORTH_DICTIONARY_SIZE 2048
#define FORTH_PROGRAM_SIZE 640*1024
#define FORTH_STACK_SIZE 256
#define FORTH_ADDR_STACK_SIZE 64
#define FORTH_I_STACK_SIZE 128
#define FORTH_COMPILE_STACK_SIZE 256
#define FORTH_MAX_VALUES 8196

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

enum {
  FORTH_PUSH,
  FORTH_DROP,
  FORTH_DUP,
  FORTH_OVER,
  FORTH_SWAP,
  FORTH_ROT,
  FORTH_CALL,
  FORTH_RET,
  FORTH_JUMP,
  FORTH_JZ,
  FORTH_LOOP,
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
  FORTH_PRINTPROGRAM,
  FORTH_I,
  FORTH_AND,
  FORTH_OR,
  FORTH_XOR,
  FORTH_INVERT,
  FORTH_NOT,
  FORTH_FUNCTION,
};

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
};

typedef struct forthInstance {
  char program[FORTH_PROGRAM_SIZE];
  int size, old_size;

  struct {
    char *name;
    int addr;
  } words[FORTH_DICTIONARY_SIZE];
  int num_words;

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

  void *values[FORTH_MAX_VALUES];
  int num_values;

  char mode, old_mode;

  int quit;
} Forth;

void forth_addFunction(Forth *fth, void (*fun)(Forth*), const char *name);

void forth_addDefaultWords(Forth *fth);

Forth *forth_newForth();
void forth_freeForth(Forth *fth);

int forth_has(Forth *fth, int n);
void *forth_pop(Forth *fth);
void forth_push(Forth *fth, void *val);

int forth_isInteger(char *s, intmax_t *n);
int forth_validIdentifier(char *s);
void forth_capitalize(char *s);

void forth_printStack(Forth *fth);
void forth_run(Forth *fth, int start);
int forth_done(Forth *fth);
void forth_printInstruction(Forth *fth, int pc);
void forth_printProgram(Forth *fth);

void forth_compileToken(Forth *fth, char *s);
void forth_doString(Forth *fth, const char *text);
void forth_doFile(Forth *fth, const char *filename);

void forth_addInstruction(Forth *fth, char ins);
void forth_addValue(Forth *fth, void *val);
void forth_addString(Forth *fth, char *s);
void *forth_getValue(Forth *fth, int addr);
void forth_setValue(Forth *fth, int addr, void *val);
int forth_getValueIndex(Forth *fth, int addr);
void forth_setValueIndex(Forth *fth, int addr, int i);

void forth_nextInstruction(Forth *fth, int *pc);
void forth_addWord(Forth *fth, char *name);
int forth_findWord(Forth *fth, char *name);
void forth_forgetWord(Forth *fth, char *name);
void forth_create(Forth *fth, char *name, void *val);

#endif
