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
#define VC   ((int)e.c)
#define RA   regs[e.a]
#define RB   regs[e.b]
#define RC   regs[e.c]
#define VBx  (unsigned short)(((VB<<8)+VC))
#define sVBx (short)(((VB<<8)+VC))

OBJ TrVM_run(VM, TrBlock *block) {
  TrInst *ip = block->code.a;
  TrInst e = *ip;
  OBJ *k = block->k.a;
  char **strings = block->strings.a;
  /* TODO alloc proper size, store in block */
  OBJ regs[10];
  OBJ locals[10];
  
#ifdef TR_THREADED_DISPATCH
  static void *labels[] = { TR_OP_LABELS };
#endif
  
  OPCODES;
    OP(NONE):       DISPATCH;
    OP(MOVE):       RA = RB; DISPATCH;
    OP(LOADK):      RA = k[VBx]; DISPATCH;
    OP(STRING):     RA = TrString_new2(vm, strings[VBx]); DISPATCH;
    OP(SEND):       RA = tr_send(RA, k[VB]); DISPATCH;
    OP(SELF):       RA = block->self; DISPATCH;
    OP(JMP):        ip += VA; DISPATCH;
    OP(JMPIF):      if (TR_TEST(RA)) ip += sVBx; DISPATCH;
    OP(JMPUNLESS):  if (!TR_TEST(RA)) ip += sVBx; DISPATCH;
    OP(SETLOCAL):   locals[VA] = RB; DISPATCH;
    OP(GETLOCAL):   RA = locals[VB]; DISPATCH;
    OP(NIL):        RA = TR_NIL; DISPATCH;
    OP(BOOL):       RA = VB+1; DISPATCH;
    OP(RETURN):     return RA;
  END_OPCODES;
}

TrVM *TrVM_new() {
  TrVM *vm = TR_ALLOC(TrVM);
  vm->symbols = kh_init(str);
  
  TrClass_init(vm);
  TrObject_init(vm);
  TR_CCLASS(TR_CLASS(Class))->super = TR_CLASS(Object);
  TrSymbol_init(vm);
  TrString_init(vm);
  TrFixnum_init(vm);
  TrArray_init(vm);
  
  return vm;
}

void TrVM_destroy(TrVM *vm) {
  /* TODO */
  kh_destroy(str, vm->symbols);
}
