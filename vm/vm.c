#include <stdio.h>
#include <assert.h>
#include "tr.h"
#include "opcode.h"
#include "internal.h"

OBJ TrVM_step(VM);

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
  if (!f->sites.a) /* HACK hu? required or can't init twice */
    kv_init(f->sites);
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

#ifdef TR_INLINE_METHOD
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

static inline OBJ TrVM_call(VM, TrFrame *f, OBJ receiver, OBJ method, int argc, OBJ *args) {
  f->method = TR_CMETHOD(method);
  if (f->method->arity == -1) {
    return f->method->func(vm, receiver, argc, args);
  } else {
    if (f->method->arity != argc) tr_raise("Expected %d arguments, got %d.\n", f->method->arity, argc);
    switch (argc) {
      case 0:  return f->method->func(vm, receiver); break;
      case 1:  return f->method->func(vm, receiver, args[0]); break;
      case 2:  return f->method->func(vm, receiver, args[0], args[1]); break;
      case 3:  return f->method->func(vm, receiver, args[0], args[1], args[2]); break;
      case 4:  return f->method->func(vm, receiver, args[0], args[1], args[2], args[3]); break;
      case 5:  return f->method->func(vm, receiver, args[0], args[1], args[2], args[3], args[4]); break;
      case 6:  return f->method->func(vm, receiver, args[0], args[1], args[2], args[3], args[4], args[5]); break;
      case 7:  return f->method->func(vm, receiver, args[0], args[1], args[2], args[3], args[4], args[5], args[6]); break;
      case 8:  return f->method->func(vm, receiver, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7]); break;
      case 9:  return f->method->func(vm, receiver, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8]); break;
      case 10: return f->method->func(vm, receiver, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9]); break;
      default: tr_raise("Too much arguments: %d, max is %d for now.\n", argc, 10);
    }
  }
}

static inline OBJ TrVM_defclass(VM, TrFrame *f, OBJ name, TrBlock *b) {
  OBJ class = TrClass_new(vm, name, TR_CLASS(Object));
  vm->cf++;
  TrFrame_init(vm, vm->cf, b);
  FRAME->self = class;
  FRAME->class = class;
  TrObject_const_set(vm, class, name, class);
  
  TrVM_step(vm);
  
  vm->cf--;
  
  return class;
}

static OBJ TrVM_interpret(VM, OBJ self) {
  TrBlock *b = (TrBlock *)TR_CMETHOD(FRAME->method)->data;
  vm->cf++;
  TrFrame_init(vm, vm->cf, b);
  FRAME->self = self;
  FRAME->class = TR_COBJECT(self)->class;
  
  TrVM_step(vm);
  
  vm->cf--;
  
  return TR_NIL;
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

OBJ TrVM_step(VM) {
  TrFrame *f = FRAME;
  TrInst *ip = f->ip;
  register TrInst e = *ip;
  OBJ *k = f->block->k.a;
  char **strings = f->block->strings.a;
  register OBJ *regs = f->regs;
  OBJ *locals = f->locals;
  TrBlock **blocks = f->block->blocks.a;
  
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
    OP(NIL):        R[A] = TR_NIL; DISPATCH;
    OP(BOOL):       R[A] = B+1; DISPATCH;
    OP(RETURN):     return R[A];
    
    /* variable and consts */
    OP(SETLOCAL):   locals[A] = R[B]; DISPATCH;
    OP(GETLOCAL):   R[A] = locals[B]; DISPATCH;
    OP(SETCONST):   TrObject_const_set(vm, f->self, k[Bx], R[A]); DISPATCH;
    OP(GETCONST):   R[A] = TrObject_const_get(vm, f->self, k[Bx]); DISPATCH;
    
    /* method calling */
    OP(LOOKUP):     R[A+1] = TrVM_lookup(vm, f, R[A], k[Bx], ip); DISPATCH;
    OP(CALL):       R[A] = TrVM_call(vm, f, R[A], R[A+1], B, &R[A+2]); DISPATCH;
    OP(CACHE):
      /* TODO how to expire cache? */
      if (SITE[C].class == TR_COBJECT(R[A])->class) {
        R[A+1] = SITE[C].method;
        ip += B;
      } else {
        /* TODO remove CallSite if too much miss, CallSite should be linked list. */
        SITE[C].miss++;
      }
      DISPATCH;
    
    /* definition */
    OP(DEF):
      TrClass_add_method(vm, f->class, k[Bx], TrMethod_new(vm, (TrFunc *)TrVM_interpret, (OBJ)blocks[A], 0));
      DISPATCH;
    OP(CLASS): R[A] = TrVM_defclass(vm, f, k[Bx], blocks[A]); DISPATCH;
    
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
  vm->consts = kh_init(OBJ);
  
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
