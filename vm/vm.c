#include <stdio.h>
#include <sys/stat.h>
#include <assert.h>
#include "tr.h"
#include "opcode.h"
#include "internal.h"

static OBJ TrVM_interpret(VM, TrFrame *f, TrBlock *b, int start, int argc, OBJ argv[], TrClosure *closure);

static inline void TrFrame_push(VM, OBJ self, OBJ class, TrClosure *closure) {
  register int cf = ++vm->cf;
  if (cf >= TR_MAX_FRAMES) tr_raise(SystemStackError, "Stack overflow");
  register TrFrame *f = FRAME;
  f->self = self;
  f->class = class;
  f->closure = closure;
}

static inline void TrFrame_pop(VM) {
  /* TODO for GC: release everything on the stack */
  vm->cf--;
}

static OBJ TrVM_lookup(VM, TrBlock *b, OBJ receiver, OBJ msg, TrInst *ip) {
  OBJ method = TrObject_method(vm, receiver, msg);
  if (!method) tr_raise(NoMethodError, "Method not found: `%s'", TR_STR_PTR(msg));

#if TR_CALL_SITE
  TrInst *boing = (ip-1);
  TrCallSite *s = (kv_pushp(TrCallSite, b->sites));
  s->class = TR_CLASS(receiver);
  s->method = method;
  s->miss = 0;
  
  /* Implement Monomorphic method cache by replacing the previous instruction (BOING)
     w/ CACHE that uses the CallSite to find the method instead of doing a full lookup. */
  SET_OPCODE(*boing, TR_OP_CACHE);
  SETARG_A(*boing, GETARG_A(*ip)); /* receiver register */
  SETARG_B(*boing, 1); /* jmp */
  SETARG_C(*boing, kv_size(b->sites)-1); /* CallSite index */
#endif
  
  return method;
}

static inline OBJ TrVM_call(VM, OBJ receiver, OBJ method, int argc, OBJ *args, int splat, TrClosure *cl) {
  /* prepare call frame */
  /* TODO do not create a call frame if calling a pure C function */
  TrFrame_push(vm, receiver, TR_CLASS(receiver), cl);
  register TrFrame *f = FRAME;
  register TrMethod *m = f->method = TR_CMETHOD(method);
  register TrFunc *func = f->method->func;
  OBJ ret = TR_NIL;
  
  /* splat last arg is needed */
  if (splat) {
    OBJ splated = args[argc-1];
    int splatedn = TR_ARRAY_SIZE(splated);
    OBJ *new_args = TR_ALLOC_N(OBJ, argc);
    TR_MEMCPY_N(new_args, args, OBJ, argc-1);
    TR_MEMCPY_N(new_args + argc-1, &TR_ARRAY_AT(splated, 0), OBJ, splatedn);
    argc += splatedn-1;
    args = new_args;
  }
  
  if (m->arity == -1) {
    ret = func(vm, receiver, argc, args);
  } else {
    if (m->arity != argc) tr_raise(ArgumentError, "Expected %d arguments, got %d.", f->method->arity, argc);
    switch (argc) {
      case 0:  ret = func(vm, receiver); break;
      case 1:  ret = func(vm, receiver, args[0]); break;
      case 2:  ret = func(vm, receiver, args[0], args[1]); break;
      case 3:  ret = func(vm, receiver, args[0], args[1], args[2]); break;
      case 4:  ret = func(vm, receiver, args[0], args[1], args[2], args[3]); break;
      case 5:  ret = func(vm, receiver, args[0], args[1], args[2], args[3], args[4]); break;
      case 6:  ret = func(vm, receiver, args[0], args[1], args[2], args[3], args[4], args[5]); break;
      case 7:  ret = func(vm, receiver, args[0], args[1], args[2], args[3], args[4], args[5], args[6]); break;
      case 8:  ret = func(vm, receiver, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7]); break;
      case 9:  ret = func(vm, receiver, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8]); break;
      case 10: ret = func(vm, receiver, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9]); break;
      default: tr_raise(ArgumentError, "Too much arguments: %d, max is %d for now.", argc, 10);
    }
  }
  TrFrame_pop(vm);
  return ret;
}

static OBJ TrVM_defclass(VM, OBJ name, TrBlock *b, int module, OBJ super) {
  OBJ mod = TrObject_const_get(vm, FRAME->class, name);
  
  if (!mod) { /* new module/class */
    if (module)
      mod = TrModule_new(vm, name);
    else
      mod = TrClass_new(vm, name, super ? super : TR_CORE_CLASS(Object));
    TrObject_const_set(vm, FRAME->class, name, mod);
  }
  TrFrame_push(vm, mod, mod, 0);
  TrVM_interpret(vm, FRAME, b, 0, 0, 0, 0);
  TrFrame_pop(vm);
  return mod;
}

static OBJ TrVM_interpret_method(VM, OBJ self, int argc, OBJ argv[]) {
  UNUSED(self);
  assert(FRAME->method);
  register TrBlock *b = (TrBlock *)TR_CMETHOD(FRAME->method)->data;
  if (argc != (int)b->argc) tr_raise(ArgumentError, "wrong number of arguments (%d for %lu)", argc, b->argc);
  return TrVM_interpret(vm, FRAME, b, 0, argc, argv, 0);
}

static OBJ TrVM_interpret_method_with_defaults(VM, OBJ self, int argc, OBJ argv[]) {
  UNUSED(self);
  assert(FRAME->method);
  register TrBlock *b = (TrBlock *)TR_CMETHOD(FRAME->method)->data;
  int req_argc = b->argc - kv_size(b->defaults);
  if (argc < req_argc) tr_raise(ArgumentError, "wrong number of arguments (%d for %d)", argc, req_argc);
  if (argc > (int)b->argc) tr_raise(ArgumentError, "wrong number of arguments (%d for %lu)", argc, b->argc);
  int defi = argc - req_argc - 1; /* index in defaults table or -1 for none */
  return TrVM_interpret(vm, FRAME, b, defi < 0 ? 0 : kv_A(b->defaults, defi), argc, argv, 0);
}

static OBJ TrVM_interpret_method_with_splat(VM, OBJ self, int argc, OBJ argv[]) {
  UNUSED(self);
  assert(FRAME->method);
  register TrBlock *b = (TrBlock *)TR_CMETHOD(FRAME->method)->data;
  /* TODO support defaults */
  assert(kv_size(b->defaults) == 0 && "defaults with splat not supported for now");
  if (argc < (int)b->argc-1) tr_raise(ArgumentError, "wrong number of arguments (%d for %lu)", argc, b->argc-1);
  argv[b->argc-1] = TrArray_new3(vm, argc - b->argc + 1, &argv[b->argc-1]);
  return TrVM_interpret(vm, FRAME, b, 0, b->argc, argv, 0);
}

static OBJ TrVM_defmethod(VM, TrFrame *f, OBJ name, TrBlock *b, int meta, OBJ receiver) {
  TrFunc *func;
  if (b->arg_splat)
    func = (TrFunc *) TrVM_interpret_method_with_splat;
  else if (kv_size(b->defaults) > 0)
    func = (TrFunc *) TrVM_interpret_method_with_defaults;
  else
    func = (TrFunc *) TrVM_interpret_method;
  OBJ method = TrMethod_new(vm, func, (OBJ)b, -1);
  if (meta)
    TrObject_add_singleton_method(vm, receiver, name, method);
  else
    TrModule_add_method(vm, f->class, name, method);
  return TR_NIL;
}

static inline OBJ TrVM_yield(VM, TrFrame *f, int argc, OBJ argv[]) {
  TrClosure *cl = f->closure;
  if (!cl) tr_raise(LocalJumpError, "no block given");
  TrFrame_push(vm, cl->self, cl->class, cl->parent);
  OBJ ret = TrVM_interpret(vm, FRAME, cl->block, 0, argc, argv, cl);
  TrFrame_pop(vm);
  return ret;
}

/* dispatch macros */
#define NEXT_INST      (i = *++ip)
#if TR_THREADED_DISPATCH
#define OPCODES        static void *labels[] = { TR_OP_LABELS }; goto *labels[OPCODE];
#define END_OPCODES    
#define OP(name)       op_##name
#define DISPATCH       NEXT_INST; goto *labels[OPCODE]
#else
#define OPCODES        for(;;) { switch(OPCODE) {
#define END_OPCODES    default: printf("unknown opcode: %d\n", (int)OPCODE); }}
#define OP(name)       case TR_OP_##name
#define DISPATCH       NEXT_INST; break
#endif

/* register access macros */
#define OPCODE GET_OPCODE(i)
#define A      GETARG_A(i)
#define B      GETARG_B(i)
#define C      GETARG_C(i)
#define nA     GETARG_A(*(ip+1))
#define nB     GETARG_B(*(ip+1))
#define R      stack
#define RK(X)  (X & (1 << (SIZE_B - 1)) ? k[X & ~0x100] : R[X])
#define Bx     GETARG_Bx(i)
#define sBx    GETARG_sBx(i)
#define SITE   (b->sites.a)

static OBJ TrVM_interpret(VM, register TrFrame *f, TrBlock *b, int start, int argc, OBJ argv[], TrClosure *closure) {
#if TR_USE_MACHINE_REGS && __i386__
  register TrInst *ip __asm__ ("esi") = b->code.a + start;
  register OBJ *stack __asm__ ("edi") = f->stack;
#elif TR_USE_MACHINE_REGS && __x86_64__
  register TrInst *ip __asm__ ("r15") = b->code.a + start;
  register OBJ *stack __asm__ ("r14") = f->stack;
#else
  register TrInst *ip = b->code.a + start;
  register OBJ *stack = f->stack;
#endif
  TrInst i = *ip;
  OBJ *k = b->k.a;
  char **strings = b->strings.a;
  TrBlock **blocks = b->blocks.a;
  f->line = b->line;
  f->filename = b->filename;
  TrUpval *upvals = closure ? closure->upvals : 0;

  /* transfer locals */
  if (argc > 0) { 
    assert(argc <= (int)kv_size(b->locals) && "can't fit args in locals");
    TR_MEMCPY_N(stack, argv, OBJ, argc);
  }
  
  OPCODES;
    
    OP(BOING):      DISPATCH;
    
    /* register loading */
    OP(MOVE):       R[A] = R[B]; DISPATCH;
    OP(LOADK):      R[A] = k[Bx]; DISPATCH;
    OP(STRING):     R[A] = TrString_new2(vm, strings[Bx]); DISPATCH;
    OP(SELF):       R[A] = f->self; DISPATCH;
    OP(NIL):        R[A] = TR_NIL; DISPATCH;
    OP(BOOL):       R[A] = B; DISPATCH;
    OP(NEWARRAY):   R[A] = TrArray_new3(vm, B, &R[A+1]); DISPATCH;
    OP(NEWHASH):    R[A] = TrHash_new2(vm, B, &R[A+1]); DISPATCH;
    OP(NEWRANGE):   R[A] = TrRange_new(vm, R[A], R[B], C); DISPATCH;
    
    /* return */
    OP(RETURN):     return R[A];
    OP(YIELD):      R[A] = TrVM_yield(vm, f, B, &R[A+1]); DISPATCH;
    
    /* variable and consts */
    OP(SETUPVAL):   assert(upvals && upvals[B].value); *(upvals[B].value) = R[A]; DISPATCH;
    OP(GETUPVAL):   assert(upvals); R[A] = *(upvals[B].value); DISPATCH;
    OP(SETIVAR):    TR_KH_SET(TR_COBJECT(f->self)->ivars, k[Bx], R[A]); DISPATCH;
    OP(GETIVAR):    R[A] = TR_KH_GET(TR_COBJECT(f->self)->ivars, k[Bx]); DISPATCH;
    OP(SETCVAR):    TR_KH_SET(TR_COBJECT(f->class)->ivars, k[Bx], R[A]); DISPATCH;
    OP(GETCVAR):    R[A] = TR_KH_GET(TR_COBJECT(f->class)->ivars, k[Bx]); DISPATCH;
    OP(SETCONST):   TrObject_const_set(vm, f->self, k[Bx], R[A]); DISPATCH;
    OP(GETCONST):   R[A] = TrObject_const_get(vm, f->self, k[Bx]); DISPATCH;
    OP(SETGLOBAL):  TR_KH_SET(vm->globals, k[Bx], R[A]); DISPATCH;
    OP(GETGLOBAL):  R[A] = TR_KH_GET(vm->globals, k[Bx]); DISPATCH;
    
    /* method calling */
    OP(LOOKUP):     R[A+1] = TrVM_lookup(vm, b, R[A], k[Bx], ip); DISPATCH;
    OP(CACHE):
      /* TODO how to expire cache? */
      assert(&SITE[C] && "Method cached but no CallSite found");
      if (likely(SITE[C].class == TR_CLASS((R[A])))) {
        R[A+1] = SITE[C].method;
        ip += B;
      } else {
        /* TODO invalidate CallSite if too much miss. */
        SITE[C].miss++;
      }
      DISPATCH;
    OP(CALL): {
      TrClosure *cl = 0;
      TrInst ci = i;
      if (C > 0) {
        /* Get upvalues using the pseudo-instructions following the CALL instruction.
           Eg.: there's one upval to a local (x) to be passed:
             call    0  0  0
             move    0  0  0 ; this is not executed
             return  0
         */
        cl = TrClosure_new(vm, blocks[C-1], f->self, f->class, f->closure);
        size_t n, nupval = kv_size(cl->block->upvals);
        for (n = 0; n < nupval; ++n) {
          NEXT_INST;
          if (OPCODE == TR_OP_MOVE) {
            cl->upvals[n].value = &R[B];
          } else {
            assert(OPCODE == TR_OP_GETUPVAL);
            cl->upvals[n].value = upvals[B].value;
          }
        }
      }
      R[GETARG_A(ci)] = TrVM_call(vm,
                            R[GETARG_A(ci)], /* receiver */
                            R[GETARG_A(ci)+1], /* method */
                            GETARG_B(ci) >> 1, &R[GETARG_A(ci)+2], /* args */
                            GETARG_B(ci) & 1, /* splat */
                            cl /* closure */
                           );
      DISPATCH;
    }
    
    /* definition */
    OP(DEF):        TrVM_defmethod(vm, f, k[Bx], blocks[A], 0, 0); DISPATCH;
    OP(METADEF):    TrVM_defmethod(vm, f, k[Bx], blocks[A], 1, R[nA]); ip++; DISPATCH;
    OP(CLASS):      TrVM_defclass(vm, k[Bx], blocks[A], 0, R[nA]); ip++; DISPATCH;
    OP(MODULE):     TrVM_defclass(vm, k[Bx], blocks[A], 1, 0); DISPATCH;
    
    /* jumps */
    OP(JMP):        ip += sBx; DISPATCH;
    OP(JMPIF):      if ( TR_TEST(R[A])) ip += sBx; DISPATCH;
    OP(JMPUNLESS):  if (!TR_TEST(R[A])) ip += sBx; DISPATCH;
    
    /* arithmetic optimizations */
    /* TODO cache lookup in tr_send and force send if method was redefined */
    #define ARITH_OPT(MSG, FUNC) {\
      OBJ rb = RK(B); \
      if (likely(TR_IS_FIX(rb))) \
        R[A] = FUNC; \
      else \
        R[A] = tr_send(rb, MSG, RK(C)); \
    }
    OP(ADD):        ARITH_OPT(vm->sADD, TR_INT2FIX(TR_FIX2INT(rb) + TR_FIX2INT(RK(C))) ); DISPATCH;
    OP(SUB):        ARITH_OPT(vm->sSUB, TR_INT2FIX(TR_FIX2INT(rb) - TR_FIX2INT(RK(C))) ); DISPATCH;
    OP(LT):         ARITH_OPT(vm->sLT, TR_BOOL(TR_FIX2INT(rb) < TR_FIX2INT(RK(C))) ); DISPATCH;
    OP(NEG):        ARITH_OPT(vm->sNEG, TR_INT2FIX(-TR_FIX2INT(rb)) ); DISPATCH;
    OP(NOT): {
      OBJ rb = RK(B);
      R[A] = TR_BOOL(!TR_TEST(rb));
      DISPATCH;
    }
  END_OPCODES;
}

void TrVM_raise(VM, OBJ exception) {
  /* Short-circuit when error before VM was started */
  if (vm->cf < 0) TrException_default_handler(vm, exception);
  
  OBJ backtrace = TrException_backtrace(vm, exception);
  tr_setglobal("$!", exception);
  tr_setglobal("$@", backtrace);
  
  TrFrame *f;
  TrFrame_pop(vm);
  for (f = FRAME; vm->cf >= 0; TrFrame_pop(vm), f = FRAME) {
    OBJ str;
    char *filename = f->filename ? TR_STR_PTR(f->filename) : "?";
    if (f->method)
      str = tr_sprintf(vm, "\tfrom %s:%lu:in `%s'",
                       filename, f->line, TR_STR_PTR(TR_CMETHOD(f->method)->name));
    else
      str = tr_sprintf(vm, "\tfrom %s:%lu",
                       filename, f->line);
    TR_ARRAY_PUSH(backtrace, str);
    
    /* TODO run rescue and ensure blocks */
  }
  
  /* not rescued, use default handler */
  TrException_default_handler(vm, exception);
}

OBJ TrVM_eval(VM, char *code, char *filename) {
  TrBlock *b = TrBlock_compile(vm, code, filename, 0);
  if (vm->debug) TrBlock_dump(vm, b);
  return TrVM_run(vm, b, vm->self, TR_CLASS(vm->self), 0, 0);
}

OBJ TrVM_load(VM, char *filename) {
  FILE *fp;
  struct stat stats;
  
  if (stat(filename, &stats) == -1) tr_raise_errno(filename);
  fp = fopen(filename, "rb");
  if (!fp) tr_raise_errno(filename);
  
  char *string = TR_ALLOC_N(char, stats.st_size + 1);
  if (fread(string, 1, stats.st_size, fp) == stats.st_size)
    return TrVM_eval(vm, string, filename);
  
  tr_raise_errno(filename);
  return TR_NIL;
}

OBJ TrVM_run(VM, TrBlock *b, OBJ self, OBJ class, int argc, OBJ argv[]) {
  TrFrame_push(vm, self, class, 0);
  OBJ ret = TrVM_interpret(vm, FRAME, b, 0, argc, argv, 0);
  TrFrame_pop(vm);
  return ret;
}

TrVM *TrVM_new() {
  GC_INIT();

  TrVM *vm = TR_ALLOC(TrVM);
  vm->symbols = kh_init(str);
  vm->globals = kh_init(OBJ);
  vm->consts = kh_init(OBJ);
  vm->debug = 0;
  
  /* bootstrap core classes,
     order is important here, so careful, mkay? */
  TrMethod_init(vm);
  TrSymbol_init(vm);
  TrModule_init(vm);
  TrClass_init(vm);
  TrObject_preinit(vm);
  TrClass *symbolc = TR_CCLASS(TR_CORE_CLASS(Symbol));
  TrClass *modulec = TR_CCLASS(TR_CORE_CLASS(Module));
  TrClass *classc = TR_CCLASS(TR_CORE_CLASS(Class));
  TrClass *methodc = TR_CCLASS(TR_CORE_CLASS(Method));
  TrClass *objectc = TR_CCLASS(TR_CORE_CLASS(Object));
  /* set proper superclass has Object is defined last */
  symbolc->super = modulec->super = methodc->super = (OBJ)objectc;
  classc->super = (OBJ)modulec;
  /* inject core classes metaclass */
  symbolc->class = TrMetaClass_new(vm, objectc->class);
  modulec->class = TrMetaClass_new(vm, objectc->class);
  classc->class = TrMetaClass_new(vm, objectc->class);
  methodc->class = TrMetaClass_new(vm, objectc->class);
  objectc->class = TrMetaClass_new(vm, objectc->class);
  
  /* Some symbols are created before Object, so make sure all have proper class. */
  TR_KH_EACH(vm->symbols, i, sym, {
    TR_COBJECT(sym)->class = (OBJ)symbolc;
  });
  
  /* bootstrap rest of core classes, order is no longer important here */
  TrObject_init(vm);
  TrError_init(vm);
  TrBinding_init(vm);
  TrPrimitive_init(vm);
  TrKernel_init(vm);
  TrString_init(vm);
  TrFixnum_init(vm);
  TrArray_init(vm);
  TrHash_init(vm);
  TrRange_init(vm);
  TrRegexp_init(vm);
  
  vm->self = TrObject_alloc(vm, 0);
  vm->cf = -1;
  
  /* cache some commonly used values */
  vm->sADD = tr_intern("+");
  vm->sSUB = tr_intern("-");
  vm->sLT = tr_intern("<");
  vm->sNEG = tr_intern("@-");
  vm->sNOT = tr_intern("!");
  
  TrVM_load(vm, "lib/boot.rb");
  
  return vm;
}

void TrVM_destroy(TrVM *vm) {
  kh_destroy(str, vm->symbols);
  GC_gcollect();
}
