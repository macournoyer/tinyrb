#include <stdio.h>
#include <assert.h>
#include "tr.h"
#include "opcode.h"

static void TrFrame_init(VM, size_t i, TrBlock *b) {
  TrFrame *f = &vm->frames[i];
  f->block = b;
  f->method = TR_NIL;
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
#define A   (e.a)
#define B   (e.b)
#define C   (e.c)
#define R   regs
#define Bx  (unsigned short)(((B<<8)+C))
#define sBx (short)(((B<<8)+C))

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
    OP(MOVE):       R[A] = R[B]; DISPATCH;
    OP(LOADK):      R[A] = k[Bx]; DISPATCH;
    OP(STRING):     R[A] = TrString_new2(vm, strings[Bx]); DISPATCH;
    OP(SELF):       R[A] = f->self; DISPATCH;
    OP(SETLOCAL):   locals[A] = R[B]; DISPATCH;
    OP(GETLOCAL):   R[A] = locals[B]; DISPATCH;
    OP(NIL):        R[A] = TR_NIL; DISPATCH;
    OP(BOOL):       R[A] = B+1; DISPATCH;
    OP(RETURN):     return R[A];
    
    /* method calling */
    OP(LOOKUP):
      R[A+1] = TrObject_method(vm, R[A], k[Bx]);
      if (!R[A+1]) tr_raise("Method not found: %s\n", TR_STR_PTR(k[Bx]));
      /* TODO replace previous instruction w/ CACHE */
      DISPATCH;
    OP(CACHE):
      /* TODO create CallSite */
      /* TODO how to expire cache? */
      DISPATCH;
    OP(CALL):
      f->method = TR_CMETHOD(R[A+1]);
      if (f->method->arity == -1) {
        R[A] = f->method->func(vm, R[A], (int)B, &R[A+2]);
      } else {
        if (f->method->arity != B) tr_raise("Expected %d arguments, got %d.\n", f->method->arity, B);
        switch (B) {
          case 0:  R[A] = f->method->func(vm, R[A]); break;
          case 1:  R[A] = f->method->func(vm, R[A], R[A+2]); break;
          case 2:  R[A] = f->method->func(vm, R[A], R[A+2], R[A+3]); break;
          case 3:  R[A] = f->method->func(vm, R[A], R[A+2], R[A+3], R[A+4]); break;
          case 4:  R[A] = f->method->func(vm, R[A], R[A+2], R[A+3], R[A+4], R[A+5]); break;
          case 5:  R[A] = f->method->func(vm, R[A], R[A+2], R[A+3], R[A+4], R[A+5], R[A+6]); break;
          case 6:  R[A] = f->method->func(vm, R[A], R[A+2], R[A+3], R[A+4], R[A+5], R[A+6], R[A+7]); break;
          case 7:  R[A] = f->method->func(vm, R[A], R[A+2], R[A+3], R[A+4], R[A+5], R[A+6], R[A+7], R[A+8]); break;
          case 8:  R[A] = f->method->func(vm, R[A], R[A+2], R[A+3], R[A+4], R[A+5], R[A+6], R[A+7], R[A+8], R[A+9]); break;
          case 9:  R[A] = f->method->func(vm, R[A], R[A+2], R[A+3], R[A+4], R[A+5], R[A+6], R[A+7], R[A+8], R[A+9], R[A+10]); break;
          case 10: R[A] = f->method->func(vm, R[A], R[A+2], R[A+3], R[A+4], R[A+5], R[A+6], R[A+7], R[A+8], R[A+9], R[A+10], R[A+11]); break;
          default: tr_raise("Too much arguments: %d, max is %d for now.\n", B, 10);
        }
      }
      DISPATCH;
      
    /* jumps */
    OP(JMP):        ip += sBx; DISPATCH;
    OP(JMPIF):      if (TR_TEST(R[A])) ip += sBx; DISPATCH;
    OP(JMPUNLESS):  if (!TR_TEST(R[A])) ip += sBx; DISPATCH;
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
