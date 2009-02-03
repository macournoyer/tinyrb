#include <stdio.h>
#include <assert.h>
#include "tr.h"
#include "opcode.h"

/* dispatch macros */
#define NEXT_OP        (++ip, e=*ip)
#ifdef TR_THREADED_DISPATCH
#define OPCODES        goto *labels[e.i]
#define END_OPCODES    
#define OP(name)       op_##name
#define DISPATCH       NEXT_OP; goto *labels[e.i]
#else
#define OPCODES        for(;;) { switch(e.i) {
#define END_OPCODES    default: printf("unknown opcode: %d\n", (int)e.i); }}
#define OP(name)       case TR_OP_##name
#define DISPATCH       NEXT_OP; break
#endif

/* register access macros */
#define VA   ((int)e.a)
#define VB   ((int)e.b)
#define RA   regs[e.a]
#define RB   regs[e.b]

OBJ tr_run(VM, TrBlock *block) {
  TrOp *ip = block->code.a;
  TrOp e = *ip;
  OBJ regs[10]; /* TODO alloc proper size, store in frame? */
  OBJ *k = block->k.a;
  /* TrFrame *frame = FRAME; */
  
#ifdef TR_THREADED_DISPATCH
  static void *labels[] = { TR_OP_LABELS };
#endif
  
  OPCODES;
    OP(NONE):       DISPATCH;
    OP(MOVE):       RA = RB; DISPATCH;
    OP(LOADK):      RA = k[VB]; DISPATCH;
    OP(SEND):       RA = tr_send(RA, k[VB]); DISPATCH;
    OP(JMP):        ip += VA; DISPATCH;
    OP(JMP_IF):     if (TR_TEST(RA)) ip += VB; DISPATCH;
    OP(JMP_UNLESS): if (!TR_TEST(RA)) ip += VB; DISPATCH;
    OP(RETURN):     return RA;
  END_OPCODES;
}

TrVM *TrVM_new() {
  TrVM *vm = TR_ALLOC(TrVM);
  vm->cf = 0;
  vm->symbols = kh_init(str);
  
  TrObject_init(vm);
  TrSymbol_init(vm);
  
  return vm;
}

void TrVM_destroy(TrVM *vm) {
  kh_destroy(str, vm->symbols);
}
