#include "sforth.h"
#include <stdint.h>

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
    /*forth_printf(fth, "%d\t", pc);
    forth_printInstruction(fth, pc);
    fth->emit('\n');
    forth_printStack(fth);
    fth->emit('\n');*/

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
      forth_printf(fth, "%s", (char*)forth_getValue(fth, pc+1));
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
      fth->emit('\n');
      break;
    case FORTH_PRINT:
      forth_printf(fth, "%jd ", (intmax_t)forth_pop(fth));
      break;
    case FORTH_SETMEM:
      (*(void**)forth_pop(fth)) = forth_pop(fth);
      break;
    case FORTH_GETMEM:
      forth_push(fth, *(void**)forth_pop(fth));
      break;
    case FORTH_EMIT:
      fth->emit((char)(intmax_t)forth_pop(fth));
      break;
    case FORTH_KEY:
      forth_push(fth, (void*)(intmax_t)fth->key());
      break;
    case FORTH_DUP:
      v1 = forth_pop(fth);
      forth_push(fth, v1);
      forth_push(fth, v1);
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
      i_stack[i_sp++] = (intmax_t)forth_pop(fth);
      i_stack[i_sp++] = (intmax_t)forth_pop(fth);
      break;
    case FORTH_LOOP:
      if(--i_stack[i_sp-1] >= i_stack[i_sp-2])
        i_sp -= 2;
      else {
        pc = (intmax_t)forth_getValue(fth, pc+1);
        continue;
      }
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
    case FORTH_BYE:
      fth->quit = 1;
      return;
    }

    forth_nextInstruction(fth, &pc);
  }
}

void forth_printInstruction(Forth *fth, int pc) {
  switch(fth->program[pc]) {
  case FORTH_PLUS: forth_printf(fth, "+"); break;
  case FORTH_MINUS: forth_printf(fth, "-"); break;
  case FORTH_MUL: forth_printf(fth, "*"); break;
  case FORTH_DIV: forth_printf(fth, "/"); break;
  case FORTH_MOD: forth_printf(fth, "mod"); break;
  case FORTH_DUP: forth_printf(fth, "dup"); break;
  case FORTH_ROT: forth_printf(fth, "rot"); break;
  case FORTH_OVER: forth_printf(fth, "over"); break;
  case FORTH_CR: forth_printf(fth, "cr"); break;
  case FORTH_INC: forth_printf(fth, "1+"); break;
  case FORTH_DEC: forth_printf(fth, "1-"); break;
  case FORTH_GREATER: forth_printf(fth, ">"); break;
  case FORTH_LESS: forth_printf(fth, "<"); break;
  case FORTH_EQUAL: forth_printf(fth, "="); break;
  case FORTH_DROP: forth_printf(fth, "drop"); break;
  case FORTH_GETMEM: forth_printf(fth, "@"); break;
  case FORTH_SETMEM: forth_printf(fth, "!"); break;
  case FORTH_PRINT: forth_printf(fth, "."); break;
  case FORTH_DO: forth_printf(fth, "do"); break;
  case FORTH_RET: forth_printf(fth, "ret"); break;
  case FORTH_HERE: forth_printf(fth, "here"); break;
  case FORTH_ALLOT: forth_printf(fth, "allot"); break;
  case FORTH_BYE: forth_printf(fth, "bye"); break;

  case FORTH_PRINTSTRING:
    forth_printf(fth, ".\"%s\"", (char*)forth_getValue(fth, pc+1)); break;
  case FORTH_PUSH:
    forth_printf(fth, "push %jd", (intmax_t)forth_getValue(fth, pc+1)); break;
  case FORTH_CALL:
    forth_printf(fth, "call %jd", (intmax_t)forth_getValue(fth, pc+1)); break;
  case FORTH_JUMP:
    forth_printf(fth, "jump %jd", (intmax_t)forth_getValue(fth, pc+1)); break;
  case FORTH_JZ:
    forth_printf(fth, "jz %jd", (intmax_t)forth_getValue(fth, pc+1)); break;
  case FORTH_LOOP:
    forth_printf(fth, "loop %d", (intmax_t)forth_getValue(fth, pc+1)); break;
  case FORTH_CREATE:
    forth_printf(fth, "create %s", (char*)forth_getValue(fth, pc+1)); break;
  case FORTH_CONSTANT:
    forth_printf(fth, "variable %s", (char*)forth_getValue(fth, pc+1)); break;
  case FORTH_VARIABLE:
    forth_printf(fth, "variable %s", (char*)forth_getValue(fth, pc+1)); break;
  case FORTH_FORGET:
    forth_printf(fth, "forget %s", (char*)forth_getValue(fth, pc+1)); break;
  }
}

void forth_printProgram(Forth *fth) {
  int pc = 0;
  int wd = 0;

  while(pc < fth->size) {
    if(wd < fth->num_words)
      if(pc == fth->words[wd].addr) {
        forth_printf(fth, "%s:\n", fth->words[wd].name);
        wd++;
      }

    forth_printf(fth, "%d\t", pc);
    forth_printInstruction(fth, pc);
    forth_printf(fth, "\n");

    forth_nextInstruction(fth, &pc);
  }
}
