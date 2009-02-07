#include <stdio.h>
#include <assert.h>
#include "tr.h"
#include "opcode.h"

static void TrFrame_init(VM, size_t i, TrBlock *b) {
  TrFrame *f = &vm->frames[i];
  f->block = b;
  f->method = TR_NIL;
  f->params = TR_NIL;
  f->regs = TR_ALLOC_N(OBJ, b->regc);
  f->locals = TR_ALLOC_N(OBJ, kv_size(b->locals));
  f->self = TR_NIL;
  f->class = TR_NIL;
  f->line = 1;
  f->ip = b->code.a;
}

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

static OBJ TrVM_step(VM) {
  TrFrame *f = FRAME;
  TrInst *ip = f->ip;
  TrInst e = *ip;
  OBJ *k = f->block->k.a;
  char **strings = f->block->strings.a;
  OBJ *regs = f->regs;
  OBJ *locals = f->locals;
  
#ifdef TR_THREADED_DISPATCH
  static void *labels[] = { TR_OP_LABELS };
#endif
  
  OPCODES;
    OP(BOING):      DISPATCH;
    
    /* register loading */
    OP(MOVE):       RA = RB; DISPATCH;
    OP(LOADK):      RA = k[VBx]; DISPATCH;
    OP(STRING):     RA = TrString_new2(vm, strings[VBx]); DISPATCH;
    OP(SELF):       RA = f->self; DISPATCH;
    OP(SETLOCAL):   locals[VA] = RB; DISPATCH;
    OP(GETLOCAL):   RA = locals[VB]; DISPATCH;
    OP(NIL):        RA = TR_NIL; DISPATCH;
    OP(BOOL):       RA = VB+1; DISPATCH;
    OP(RETURN):     return RA;
    
    /* method calling */
    OP(LOOKUP):
      RA = TR_BOX(RA);
      regs[e.a+1] = TrObject_method(vm, RA, k[VB]);
      if (!regs[e.a+1]) tr_raise("Method not found: %s\n", TR_STR_PTR(k[VB]));
      /* TODO replace previous instruction w/ CACHE */
      DISPATCH;
    OP(CACHE):
      /* TODO how to expire cache? */
      RA = TR_BOX(RA);
      if (TR_TYPE(RA) == VC) ip += VB;
      DISPATCH;
    OP(CALL):
      f->method = TR_CMETHOD(regs[e.a+1]);
      /* TODO set f->args */
      RA = f->method->func(vm, RA);
      DISPATCH;
      
    /* jumps */
    OP(JMP):        ip += VA; DISPATCH;
    OP(JMPIF):      if (TR_TEST(RA)) ip += sVBx; DISPATCH;
    OP(JMPUNLESS):  if (!TR_TEST(RA)) ip += sVBx; DISPATCH;
  END_OPCODES;
}

OBJ TrVM_run(VM, TrBlock *b) {
  TrFrame_init(vm, 0, b);
  vm->cf = 0;
  FRAME->self = TrObject_new(vm);
  FRAME->class = TR_CLASS(Object);
  return TrVM_step(vm);
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
