#include <stdio.h>
#include <assert.h>
#include "tr.h"
#include "opcode.h"
#include "internal.h"

static void TrFrame_init(VM, size_t i, TrBlock *b) {
  TrFrame *f = &vm->frames[i];
  f->block = b;
  f->method = TR_NIL;
  f->regs = TR_ALLOC_N(OBJ, b->regc);
  f->locals = TR_ALLOC_N(OBJ, kv_size(b->locals));
  f->self = TR_NIL;
  f->class = TR_NIL;
  kv_init(f->sites);
  f->line = 1;
  f->ip = b->code.a;
}

static inline OBJ TrVM_lookup(VM, TrFrame *f, OBJ receiver, OBJ msg, TrInst *ip) {
  OBJ method = TrObject_method(vm, receiver, msg);
  if (!method) tr_raise("Method not found: %s\n", TR_STR_PTR(msg));
  TrCallSite *s = (kv_pushp(TrCallSite, f->sites));
  /* TODO support metaclass */
  s->class = TR_COBJECT(receiver)->class;
  s->method = method;
  s->miss = 0;
  /* Implement Monomorphic methoc cache by replacing the previous instruction (BOING)
     w/ CACHE that uses the CallSite to find the method instead of doing a full lookup. */
  TrInst *cache = (ip-1);
  cache->i = TR_OP_CACHE;
  cache->a = ip->a; /* receiver */
  cache->b = 1; /* jmp */
  cache->c = kv_size(f->sites)-1; /* CallSite index */

#if 0  
  /* TODO find an unintrusive way to do this, how to fallback to lookup/call? */
  /* try to inline the method as an instruction if possible */
  do { ip++; } while (ip->i != TR_OP_CALL);
  if (TR_CMETHOD(method)->func == (TrFunc *)TrFixnum_add)
    ip->i = TR_OP_FIXNUM_ADD;
  else if (TR_CMETHOD(method)->func == (TrFunc *)TrFixnum_lt)
    ip->i = TR_OP_FIXNUM_LT;
#endif
  
  return method;
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
#define A    (e.a)
#define B    (e.b)
#define C    (e.c)
#define R    regs
#define Bx   (unsigned short)(((B<<8)+C))
#define sBx  (short)(((B<<8)+C))
#define SITE (f->sites.a)

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
      R[A+1] = TrVM_lookup(vm, f, R[A], k[Bx], ip);
      DISPATCH;
    OP(CACHE):
      /* TODO how to expire cache? */
      if (SITE[C].class == TR_COBJECT(R[A])->class) {
        R[A+1] = SITE[C].method;
        ip += B;
      } else {
        /* TODO remove CallSite if too much miss */
        SITE[C].miss++;
      }
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
    
    /* optimizations */
    OP(FIXNUM_ADD): R[A] = TrFixnum_new(vm, TR_FIX2INT(R[A]) + TR_FIX2INT(R[A+2])); DISPATCH;
    OP(FIXNUM_LT):  R[A] = TR_BOOL(TR_FIX2INT(R[A]) < TR_FIX2INT(R[A+2])); DISPATCH;
    
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
  GC_INIT();

  TrVM *vm = TR_ALLOC(TrVM);
  vm->symbols = kh_init(str);
  
  /* bootstrap classes */
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
  kh_destroy(str, vm->symbols);
  GC_gcollect();
}
