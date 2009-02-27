#include <stdio.h>
#include <sys/stat.h>
#include <assert.h>
#include "tr.h"
#include "opcode.h"
#include "internal.h"

OBJ TrVM_step(VM, TrFrame *f, TrBlock * b, int argc, OBJ argv[]);

static void TrFrame_push(VM, OBJ self, OBJ class, TrBlock *block) {
  TrFrame *prevf = vm->cf < 0 ? 0 : FRAME;
  vm->cf++;
  if (vm->cf >= TR_MAX_FRAMES) tr_raise("Stack overflow");
  TrFrame *f = FRAME;
  f->method = TR_NIL;
  f->filename = TR_NIL;
  f->self = self;
  f->class = class;
  f->line = 1;
  if (prevf) {
    f->block = prevf->block;
    TR_MEMCPY(&f->rescue_jmp, &prevf->rescue_jmp, jmp_buf);
  }
  if (block) f->block = block;
  
  /* init first frame */
  if (vm->cf == 0) {
    TR_RESCUE({
      exit(1);
    })
  }
}

static void TrFrame_pop(VM) {
  vm->cf--;
}

static OBJ TrVM_lookup(VM, TrBlock *b, OBJ receiver, OBJ msg, TrInst *ip) {
  OBJ method = TrObject_method(vm, receiver, msg);
  if (!method) tr_raise("Method not found: `%s'\n", TR_STR_PTR(msg));

#ifdef TR_CALL_SITE
  TrInst *boing = (ip-1);
  TrCallSite *s = (kv_pushp(TrCallSite, b->sites));
  /* TODO support metaclass */
  s->class = TR_COBJECT(receiver)->class;
  s->method = method;
  s->miss = 0;
  
#ifdef TR_INLINE_METHOD
  #define DEF_INLINE(F, OP) \
    if (func == (TrFunc*)(F)) { \
      boing->i = TR_OP_##OP; \
      boing->a = ip->a; \
      boing->b = 2; \
      return method; \
    }
  TrFunc *func = TR_CMETHOD(method)->func;
  /* try to inline the method as an instruction if possible */
  DEF_INLINE(TrFixnum_add, FIXNUM_ADD)
  else
  DEF_INLINE(TrFixnum_sub, FIXNUM_SUB)
  else
  DEF_INLINE(TrFixnum_lt, FIXNUM_LT)
  #undef DEF_INLINE
#endif

  /* Implement Monomorphic method cache by replacing the previous instruction (BOING)
     w/ CACHE that uses the CallSite to find the method instead of doing a full lookup. */
  boing->i = TR_OP_CACHE;
  boing->a = ip->a; /* receiver register */
  boing->b = 1; /* jmp */
  boing->c = kv_size(b->sites)-1; /* CallSite index */
#endif
  
  return method;
}

static inline OBJ TrVM_call(VM, TrFrame *callingf, OBJ receiver, OBJ method, int argc, OBJ *args, TrBlock *b) {
  TrFrame_push(vm, receiver, TR_COBJECT(receiver)->class, b);
  register TrFrame *f = FRAME;
  f->method = TR_CMETHOD(method);
  register TrFunc *func = f->method->func;
  if (b) b->frame = callingf;
  OBJ ret = TR_NIL;
  if (f->method->arity == -1) {
    ret = func(vm, receiver, argc, args);
  } else {
    if (f->method->arity != argc) tr_raise("Expected %d arguments, got %d.\n", f->method->arity, argc);
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
      default: tr_raise("Too much arguments: %d, max is %d for now.\n", argc, 10);
    }
  }
  TrFrame_pop(vm);
  return ret;
}

static OBJ TrVM_defclass(VM, TrFrame *f, OBJ name, TrBlock *b, int module, OBJ super) {
  OBJ mod = TrObject_const_get(vm, FRAME->class, name);
  
  if (!mod) { /* new module/class */
    if (module)
      mod = TrModule_new(vm, name);
    else
      mod = TrClass_new(vm, name, super ? super : TR_CLASS(Object));
    TrObject_const_set(vm, FRAME->class, name, mod);
  }
  TrFrame_push(vm, mod, mod, 0);
  TrVM_step(vm, FRAME, b, 0, 0);
  TrFrame_pop(vm);
  return mod;
}

static OBJ TrVM_interpret_method(VM, OBJ self, int argc, OBJ argv[]) {
  assert(FRAME->method);
  register TrBlock *b = (TrBlock *)TR_CMETHOD(FRAME->method)->data;
  if (argc != b->argc) tr_raise("Expected %lu arguments, got %d.\n", b->argc, argc);
  return TrVM_step(vm, FRAME, b, argc, argv);
}

static OBJ TrVM_interpret_method_with_splat(VM, OBJ self, int argc, OBJ argv[]) {
  assert(FRAME->method);
  register TrBlock *b = (TrBlock *)TR_CMETHOD(FRAME->method)->data;
  if (argc < b->argc-1) tr_raise("Expected at least %lu arguments, got %d.\n", b->argc-1, argc);
  argv[b->argc-1] = TrArray_new3(vm, argc - b->argc + 1, &argv[b->argc-1]);
  return TrVM_step(vm, FRAME, b, b->argc, argv);
}

static OBJ TrVM_defmethod(VM, TrFrame *f, OBJ name, TrBlock *b) {
  TrFunc *func = (TrFunc *) (b->arg_splat
    ? TrVM_interpret_method_with_splat
    : TrVM_interpret_method);
  TrModule_add_method(vm, f->class, name, TrMethod_new(vm, func, (OBJ)b, -1));
  return TR_NIL;
}

static inline OBJ TrVM_yield(VM, TrFrame *f, TrBlock *b, int argc, OBJ argv[]) {
  if (!b) tr_raise("LocalJumpError: no block given");
  return TrVM_step(vm, b->frame, b, argc, argv);
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
#define SITE (b->sites.a)

OBJ TrVM_step(VM, register TrFrame *f, TrBlock *b, int argc, OBJ argv[]) {
  f->line = b->line;
  f->filename = b->filename;
  register TrInst *ip = b->code.a;
  register TrInst e = *ip;
  OBJ *k = b->k.a;
  char **strings = b->strings.a;
  TrBlock **blocks = b->blocks.a;
  register OBJ *regs = TR_ALLOC_N(OBJ, b->regc);
  /* transfer locals */
  OBJ *locals = f->locals = TR_ALLOC_N(OBJ, kv_size(b->locals)+argc);
  size_t i;
  for (i = 0; i < argc; ++i) locals[i] = argv[i];
  
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
    OP(NEWARRAY):   R[A] = TrArray_new3(vm, B, &R[A+1]); DISPATCH;
    OP(NEWHASH):    R[A] = TrHash_new2(vm, B, &R[A+1]); DISPATCH;
    OP(NEWRANGE):   R[A] = TrRange_new(vm, R[A], R[B], C); DISPATCH;
    
    /* return */
    OP(RETURN):     return R[A];
    OP(YIELD):      R[A] = TrVM_yield(vm, f, f->block, B, &R[A+1]); DISPATCH;
    
    /* variable and consts */
    OP(SETLOCAL):   locals[A] = R[B]; DISPATCH;
    OP(GETLOCAL):   R[A] = locals[B]; DISPATCH;
    OP(SETIVAR):    TR_SETIVAR(f->self, k[Bx], R[A]); DISPATCH;
    OP(GETIVAR):    R[A] = TR_GETIVAR(f->self, k[Bx]); DISPATCH;
    OP(SETCVAR):    TR_SETIVAR(f->class, k[Bx], R[A]); DISPATCH;
    OP(GETCVAR):    R[A] = TR_GETIVAR(f->class, k[Bx]); DISPATCH;
    OP(SETCONST):   TrObject_const_set(vm, f->self, k[Bx], R[A]); DISPATCH;
    OP(GETCONST):   R[A] = TrObject_const_get(vm, f->self, k[Bx]); DISPATCH;
    OP(SETGLOBAL):  TR_KH_SET(vm->globals, k[Bx], R[A]); DISPATCH;
    OP(GETGLOBAL):  R[A] = TR_KH_GET(vm->globals, k[Bx]); DISPATCH;
    
    /* method calling */
    OP(LOOKUP):     R[A+1] = TrVM_lookup(vm, b, R[A], k[Bx], ip); DISPATCH;
    OP(CALL):       R[A] = TrVM_call(vm, f, R[A], R[A+1], B, &R[A+2], C>0 ? blocks[C-1] : 0); DISPATCH;
    OP(CACHE):
      /* TODO how to expire cache? */
      assert(&SITE[C] && "Method cached but no CallSite found");
      if (likely(SITE[C].class == TR_COBJECT(R[A])->class)) {
        R[A+1] = SITE[C].method;
        ip += B;
      } else {
        /* TODO invalidate CallSite if too much miss. */
        SITE[C].miss++;
      }
      DISPATCH;
    
    /* definition */
    OP(DEF):    R[0] = TrVM_defmethod(vm, f, k[Bx], blocks[A]); DISPATCH;
    OP(CLASS):  R[0] = TrVM_defclass(vm, f, k[Bx], blocks[A], 0, R[0]); DISPATCH;
    OP(MODULE): R[0] = TrVM_defclass(vm, f, k[Bx], blocks[A], 1, 0); DISPATCH;
    
    /* jumps */
    OP(JMP):        ip += sBx; DISPATCH;
    OP(JMPIF):      if (TR_TEST(R[A])) ip += sBx; DISPATCH;
    OP(JMPUNLESS):  if (!TR_TEST(R[A])) ip += sBx; DISPATCH;
    
    /* optimizations */
    #define INLINE_FUNC(FNC) if (likely(SITE[C].class == TR_COBJECT(R[A])->class)) { FNC; ip += B; } else { SITE[C].miss++; }
    OP(FIXNUM_ADD):
      INLINE_FUNC(R[A] = TrFixnum_new(vm, TR_FIX2INT(R[A]) + TR_FIX2INT(R[A+2])));
      DISPATCH;
    OP(FIXNUM_SUB):
      INLINE_FUNC(R[A] = TrFixnum_new(vm, TR_FIX2INT(R[A]) - TR_FIX2INT(R[A+2])));
      DISPATCH;
    OP(FIXNUM_LT):
      INLINE_FUNC(R[A] = TR_BOOL(TR_FIX2INT(R[A]) < TR_FIX2INT(R[A+2])));
      DISPATCH;
    
  END_OPCODES;
}

OBJ TrVM_eval(VM, char *code, char *filename) {
  TrBlock *b = TrBlock_compile(vm, code, filename, 0);
  if (vm->debug) TrBlock_dump(vm, b);
  return TrVM_run(vm, b, vm->self, TR_COBJECT(vm->self)->class, 0, 0, 0);
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

void TrVM_raise(VM, OBJ exception) {
  if (vm->debug) assert(0);
  vm->exception = exception;
  if (vm->cf == -1)
    TrVM_rescue(vm);
  else
    longjmp(FRAME->rescue_jmp, 1);
}

void TrVM_rescue(VM) {
  printf("%s\n", TR_STR_PTR(vm->exception));
  
  /* Error before VM was started, can be bad... */
  if (vm->cf == -1) exit(1);
  
  size_t i;
  for (i = vm->cf-1; i != -1 ; --i) {
    TrFrame f = vm->frames[i];
    printf("\tfrom %s:%lu", f.filename ? TR_STR_PTR(f.filename) : "?", f.line);
    if (f.method) printf(":in `%s'", TR_STR_PTR(TR_CMETHOD(f.method)->name));
    printf("\n");
  }
}

OBJ TrVM_run(VM, TrBlock *b, OBJ self, OBJ class, int argc, OBJ argv[], TrBlock *block) {
  TrFrame_push(vm, self, class, block);
  OBJ ret = TrVM_step(vm, FRAME, b, argc, argv);
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
  
  /* bootstrap core classes */
  TrMethod_init(vm);
  TrSymbol_init(vm);
  TrModule_init(vm);
  TrClass_init(vm);
  TrObject_init(vm);
  TrClass *symbolc = TR_CCLASS(TR_CLASS(Symbol));
  TrClass *modulec = TR_CCLASS(TR_CLASS(Module));
  TrClass *classc = TR_CCLASS(TR_CLASS(Class));
  TrClass *methodc = TR_CCLASS(TR_CLASS(Method));
  TrClass *objectc = TR_CCLASS(TR_CLASS(Object));
  symbolc->super = modulec->super = methodc->super = (OBJ)objectc;
  classc->super = (OBJ)modulec;
  symbolc->class = modulec->class = classc->class = methodc->class = objectc->class = (OBJ)classc;
  
  /* Some symbols are created before Object, so make sure all have proper class. */
  TR_KH_EACH(vm->symbols, i, sym, {
    TR_COBJECT(sym)->class = (OBJ)symbolc;
  });
  
  TrBinding_init(vm);
  TrPrimitive_init(vm);
  TrKernel_init(vm);
  TrString_init(vm);
  TrFixnum_init(vm);
  TrArray_init(vm);
  TrHash_init(vm);
  TrRange_init(vm);
  
  vm->self = TrObject_new(vm);
  vm->cf = -1;
  
  TrVM_load(vm, "lib/boot.rb");
  
  return vm;
}

void TrVM_destroy(TrVM *vm) {
  kh_destroy(str, vm->symbols);
  GC_gcollect();
}
