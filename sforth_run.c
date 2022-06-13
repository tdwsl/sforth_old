#include "sforth.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

void forth_movePC(int *addr_stack, int addr_sp, int *pc,
    int start, int offset)
{
  for(int i = 0; i < addr_sp; i++)
    if(addr_stack[i] >= start)
      addr_stack[i] += offset;

  if(*pc >= start)
    *pc += offset;
}

void forth_run(Forth *fth, int start) {
  int pc = start;

  int addr_stack[FORTH_ADDR_STACK_SIZE];
  int addr_sp = 0;

  int i_stack[FORTH_I_STACK_SIZE];
  int i_sp = 0;

  void *v1;
  int size;

  while(pc < fth->size && !fth->quit) {
    /* trace */
    if(fth->trace) {
      printf("%d\t", pc);
      forth_printInstruction(fth, pc);
      printf("\n");
      forth_printStack(fth);
      printf("\n");
    }

    switch(fth->program[pc]) {
    case FORTH_CALL:
      addr_stack[addr_sp++] = pc;
      pc = (intmax_t)forth_getValue(fth, pc+1);
      continue;
    case FORTH_RET:
      pc = addr_stack[--addr_sp];
      break;
    case FORTH_JUMP:
      pc = (intmax_t)forth_getValue(fth, pc+1);
      continue;
    case FORTH_JZ:
      if(!(intmax_t)forth_pop(fth)) {
        pc = (intmax_t)forth_getValue(fth, pc+1);
        continue;
      }
      break;
    case FORTH_PUSH:
      forth_push(fth, forth_getValue(fth, pc+1));
      break;
    case FORTH_PRINTSTRING:
      printf("%s", (char*)forth_getValue(fth, pc+1));
      break;
    case FORTH_PLUS:
      forth_push(fth,
          (void*)((intmax_t)forth_pop(fth)+(intmax_t)forth_pop(fth)));
      break;
    case FORTH_MUL:
      forth_push(fth,
          (void*)((intmax_t)forth_pop(fth)*(intmax_t)forth_pop(fth)));
      break;
    case FORTH_MOD:
      v1 = forth_pop(fth);
      forth_push(fth, (void*)((intmax_t)forth_pop(fth)%(intmax_t)v1));
      break;
    case FORTH_DIV:
      v1 = forth_pop(fth);
      forth_push(fth, (void*)((intmax_t)forth_pop(fth)/(intmax_t)v1));
      break;
    case FORTH_MINUS:
      v1 = forth_pop(fth);
      forth_push(fth, (void*)((intmax_t)forth_pop(fth)-(intmax_t)v1));
      break;
    case FORTH_CR:
      printf("\n");
      break;
    case FORTH_PRINT:
      printf("%jd ", (intmax_t)forth_pop(fth));
      break;
    case FORTH_SETMEM:
      v1 = forth_pop(fth);
      memcpy((void**)forth_pop(fth), &v1, sizeof(void*));
      break;
    case FORTH_GETMEM:
      forth_push(fth, *(void**)forth_pop(fth));
      break;
    case FORTH_DUP:
      v1 = forth_pop(fth);
      forth_push(fth, v1);
      forth_push(fth, v1);
      break;
    case FORTH_SWAP:
      if(forth_has(fth, 2)) {
        v1 = fth->stack[fth->sp-1];
        fth->stack[fth->sp-1] = fth->stack[fth->sp-2];
        fth->stack[fth->sp-2] = v1;
      }
      break;
    case FORTH_OVER:
      if(forth_has(fth, 2))
        forth_push(fth, fth->stack[fth->sp-2]);
      else
        forth_push(fth, 0);
      break;
    case FORTH_ROT:
      if(!forth_has(fth, 3))
        break;
      v1 = fth->stack[fth->sp-3];
      fth->stack[fth->sp-3] = fth->stack[fth->sp-2];
      fth->stack[fth->sp-2] = fth->stack[fth->sp-1];
      fth->stack[fth->sp-1] = v1;
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
      forth_push(fth, (void*)(intmax_t)(forth_pop(fth) > v1));
      break;
    case FORTH_LESS:
      v1 = forth_pop(fth);
      forth_push(fth, (void*)(intmax_t)(forth_pop(fth) < v1));
      break;
    case FORTH_EQUAL:
      forth_push(fth, (void*)(intmax_t)(forth_pop(fth) == forth_pop(fth)));
      break;
    case FORTH_DROP:
      forth_pop(fth);
      break;
    case FORTH_DO:
      i_sp += 2;
      i_stack[i_sp-1] = (intmax_t)forth_pop(fth);
      i_stack[i_sp-2] = (intmax_t)forth_pop(fth);
      break;
    case FORTH_LOOP:
      if(++i_stack[i_sp-1] >= i_stack[i_sp-2])
        i_sp -= 2;
      else {
        pc = (intmax_t)forth_getValue(fth, pc+1);
        continue;
      }
      break;
    case FORTH_I:
      forth_push(fth, (void*)(intmax_t)i_stack[i_sp-1]);
      break;
    case FORTH_HERE:
      forth_push(fth, fth->here);
      break;
    case FORTH_ALLOT:
      fth->here = (void*)((intmax_t)fth->here + (intmax_t)forth_pop(fth));
      break;
    case FORTH_CREATE:
      size = fth->old_size;
      forth_create(fth, (void*)forth_getValue(fth, pc+1), fth->here);
      forth_movePC(addr_stack, addr_sp, &pc, size, fth->old_size-size);
      break;
    case FORTH_VARIABLE:
      size = fth->old_size;
      forth_create(fth, (char*)forth_getValue(fth, pc+1), fth->here);
      forth_movePC(addr_stack, addr_sp, &pc, size, fth->old_size-size);
      fth->here += sizeof(void*);
      break;
    case FORTH_CONSTANT:
      size = fth->old_size;
      forth_create(fth, (char*)forth_getValue(fth, pc+1), forth_pop(fth));
      forth_movePC(addr_stack, addr_sp, &pc, size, fth->old_size-size);
      break;
    case FORTH_FORGET:
      size = fth->old_size;
      forth_forgetWord(fth, (char*)forth_getValue(fth, pc+1));
      forth_movePC(addr_stack, addr_sp, &pc, size, fth->old_size-size);
      break;
    case FORTH_INCLUDE:
      size = fth->old_size;
      fth->size = size;
      forth_doFile(fth, (char*)forth_getValue(fth, pc+1));
      forth_movePC(addr_stack, addr_sp, &pc, size, fth->old_size-size);
      break;
    case FORTH_BYE:
      fth->quit = 1;
      return;
    case FORTH_PRINTPROGRAM:
      forth_printProgram(fth);
      break;
    case FORTH_AND:
      forth_push(fth,
          (void*)((intmax_t)forth_pop(fth)&(intmax_t)forth_pop(fth)));
      break;
    case FORTH_OR:
      forth_push(fth,
          (void*)((intmax_t)forth_pop(fth)|(intmax_t)forth_pop(fth)));
      break;
    case FORTH_XOR:
      forth_push(fth,
          (void*)((intmax_t)forth_pop(fth)^(intmax_t)forth_pop(fth)));
      break;
    case FORTH_NOT:
      if(forth_has(fth, 1))
        fth->stack[fth->sp-1] =
            (void*)((intmax_t)!(intmax_t)fth->stack[fth->sp-1]);
      break;
    case FORTH_INVERT:
      if(forth_has(fth, 1))
        fth->stack[fth->sp-1] = (void*)(~(intmax_t)fth->stack[fth->sp-1]);
      break;
    case FORTH_FUNCTION:
      ((void (*)(Forth*))forth_getValue(fth, pc+1))(fth);
      break;
    case FORTH_TRACEON:
      fth->trace = 1;
      break;
    case FORTH_TRACEOFF:
      fth->trace = 0;
      break;
    }

    forth_nextInstruction(fth, &pc);
  }
}

void forth_printInstruction(Forth *fth, int pc) {
  switch(fth->program[pc]) {
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
  case FORTH_DROP: printf("drop"); break;
  case FORTH_GETMEM: printf("@"); break;
  case FORTH_SETMEM: printf("!"); break;
  case FORTH_PRINT: printf("."); break;
  case FORTH_DO: printf("do"); break;
  case FORTH_RET: printf("ret"); break;
  case FORTH_HERE: printf("here"); break;
  case FORTH_ALLOT: printf("allot"); break;
  case FORTH_BYE: printf("bye"); break;
  case FORTH_PRINTPROGRAM: printf("printprogram"); break;
  case FORTH_I: printf("i"); break;
  case FORTH_AND: printf("and"); break;
  case FORTH_OR: printf("or"); break;
  case FORTH_XOR: printf("xor"); break;
  case FORTH_INVERT: printf("invert"); break;
  case FORTH_NOT: printf("not"); break;
  case FORTH_SWAP: printf("swap"); break;
  case FORTH_TRACEON: printf("traceon"); break;
  case FORTH_TRACEOFF: printf("traceoff"); break;

  case FORTH_PRINTSTRING:
    printf(".\"%s\"", (char*)forth_getValue(fth, pc+1)); break;
  case FORTH_INCLUDE:
    printf("include %s", (char*)forth_getValue(fth, pc+1)); break;
  case FORTH_PUSH:
    printf("push %jd", (intmax_t)forth_getValue(fth, pc+1)); break;
  case FORTH_CALL:
    {
      int i;
      intmax_t a = (intmax_t)forth_getValue(fth, pc+1);
      for(i = 0; i < fth->num_words; i++)
        if(fth->words[i].addr == a)
          break;
      printf("call %s", fth->words[i].name);
    }
    break;
  case FORTH_JUMP:
    printf("jump %jd", (intmax_t)forth_getValue(fth, pc+1)); break;
  case FORTH_JZ:
    printf("jz %jd", (intmax_t)forth_getValue(fth, pc+1)); break;
  case FORTH_LOOP:
    printf("loop %jd", (intmax_t)forth_getValue(fth, pc+1)); break;
  case FORTH_CREATE:
    printf("create %s", (char*)forth_getValue(fth, pc+1)); break;
  case FORTH_CONSTANT:
    printf("variable %s", (char*)forth_getValue(fth, pc+1)); break;
  case FORTH_VARIABLE:
    printf("variable %s", (char*)forth_getValue(fth, pc+1)); break;
  case FORTH_FORGET:
    printf("forget %s", (char*)forth_getValue(fth, pc+1)); break;
  case FORTH_FUNCTION:
    printf("function %jd", (uintmax_t)forth_getValue(fth, pc+1)); break;
  }
}

void forth_printProgram(Forth *fth) {
  int pc = 0;
  int wd = 0;

  while(pc < fth->size) {
    if(wd < fth->num_words)
      if(pc == fth->words[wd].addr) {
        printf("%s:\n", fth->words[wd].name);
        wd++;
      }

    printf("%d\t", pc);
    forth_printInstruction(fth, pc);
    printf("\n");

    forth_nextInstruction(fth, &pc);
  }
}
