#ifndef _TINYRB_H_
#define _TINYRB_H_

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <assert.h>
#include "kvec.h"
#include "khash.h"
#include "config.h"

#define TR_ALLOC(T)          (T *)malloc(sizeof(T))
#define TR_ALLOC_N(T,N)      (T *)malloc(sizeof(T)*(N))

#define TR_MEMZERO(X,T)      memset((X), 0, sizeof(T))
#define TR_MEMZERO_N(X,T,N)  memset((X), 0, sizeof(T)*(N))
#define TR_MEMCPY(X,Y,T)     memcpy((X), (Y), sizeof(T))
#define TR_MEMCPY_N(X,Y,T,N) memcpy((X), (Y), sizeof(T)*(N))

#define TR_STR_PTR(S)        (((TrString*)S)->ptr)
#define TR_COBJECT(X)        ((TrObject*)X)
#define TR_CCLASS(X)         ((TrClass*)X)
#define TR_CMETHOD(X)        ((TrMethod*)X)

#define VM                   TrVM *vm
#define FRAME                &vm->frames[vm->cf]

#define TR_NIL               ((OBJ)0)
#define TR_TRUE              ((OBJ)1)
#define TR_FALSE             ((OBJ)2)
#define TR_TEST(X)           ((X) == TR_NIL || (X) == TR_FALSE ? 0 : 1)
#define TR_BOOL(X)           ((X) ? TR_TRUE : TR_FALSE)

#define TR_OBJECT_HEADER \
  TR_T type; \
  OBJ class; \
  OBJ metaclass; \
  OBJ ivars
#define TR_INIT_OBJ(T) ({ \
  Tr##T *o = TR_ALLOC(Tr##T); \
  o->type  = TR_T_##T; \
  o->class = vm->classes[TR_T_##T]; \
  o; \
})
#define TR_CLASS(T)           vm->classes[TR_T_##T]
#define TR_INIT_CLASS(T,S)    TR_CLASS(T) = TrClass_new(vm, tr_intern(#T), TR_CLASS(S))

#define tr_intern(S)         TrSymbol_new(vm, (S))
#define tr_raise(M,A...)     (printf("Error: "), printf(M, ##A), assert(0))
#define tr_def(C,N,F)        TrClass_add_method(vm, (C), tr_intern(N), TrMethod_new(vm, (TrFunc *)(F), TR_NIL))
#define tr_send(R,MSG,A...)  ({ \
  TrMethod *m = TR_CMETHOD(TrObject_method(vm, (R), (MSG))); \
  if (!m) tr_raise("Method not found: %s\n", TR_STR_PTR(MSG)); \
  vm->method = (OBJ)m; \
  m->func(vm, (R), ##A); \
})
#define tr_send2(R,STR,A...) tr_send((R), tr_intern(STR), ##A)

typedef unsigned long OBJ;

KHASH_MAP_INIT_STR(str, OBJ);
KHASH_MAP_INIT_INT(OBJ, OBJ);

typedef enum {
  TR_T_Object, TR_T_Class, TR_T_Method, TR_T_Symbol, TR_T_String,
  TR_T_MAX /* keep last */
} TR_T;

struct TrBlock;

typedef struct {
  khash_t(str) *symbols;
  OBJ classes[TR_T_MAX];
  OBJ method;      /* current called method */
  struct TrBlock *block;  /* current block */
} TrVM;

typedef struct {
  TR_OBJECT_HEADER;
} TrObject;

typedef struct {
  TR_OBJECT_HEADER;
  OBJ name;
  OBJ super;
  khash_t(OBJ) *methods;
} TrClass;

typedef OBJ (TrFunc)(VM, OBJ receiver, ...);
typedef struct {
  TR_OBJECT_HEADER;
  TrFunc *func;
  OBJ data;
} TrMethod;

typedef struct {
  TR_OBJECT_HEADER;
  char *ptr;
  size_t len;
  unsigned char interned:1;
} TrString;
typedef TrString TrSymbol;

typedef struct {
  unsigned char i, a, b, c;
} TrInst;

typedef struct TrBlock {
  /* static, at compile time */
  kvec_t(OBJ) k; /* TODO rename to values ? */
  kvec_t(char *) strings;
  kvec_t(OBJ) locals; /* TODO rename to local_names */
  kvec_t(TrInst) code;
  kvec_t(struct TrBlock *) blocks;
  size_t regc;
  size_t next_reg; /* next register to use */
  
  /* dynamic, used at runtime */
  OBJ *regs;
  /* OBJ *locals; */
  OBJ self;
  OBJ class;
} TrBlock;

typedef struct {
  int curline;
  TrVM *vm;
  TrBlock *block;
  size_t reg;
} TrCompiler;

/* vm */
TrVM *TrVM_new();
void TrVM_destroy(TrVM *vm);
OBJ tr_run(VM, TrBlock *code);

/* string */
OBJ TrSymbol_new(VM, const char *str);
OBJ TrString_new(VM, const char *str, size_t len);
OBJ TrString_new2(VM, const char *str);
void TrSymbol_init(VM);
void TrString_init(VM);

/* object */
OBJ TrObject_method(VM, OBJ self, OBJ name);
void TrObject_init(VM);

/* class */
OBJ TrClass_new(VM, OBJ name, OBJ super);
OBJ TrClass_lookup(VM, OBJ self, OBJ name);
OBJ TrClass_add_method(VM, OBJ self, OBJ name, OBJ method);
void TrClass_init(VM);
OBJ TrMethod_new(VM, TrFunc *func, OBJ data);

/* compiler */
TrCompiler *TrCompiler_new(VM, const char *fn);
void TrCompiler_dump(TrCompiler *c);
int TrCompiler_call(TrCompiler *c, OBJ msg);
int TrCompiler_pushk(TrCompiler *c, OBJ k);
int TrCompiler_string(TrCompiler *c, OBJ str);
int TrCompiler_setlocal(TrCompiler *c, OBJ name, int reg);
void TrCompiler_finish(TrCompiler *c);
void TrCompiler_destroy(TrCompiler *c);

/* parser */
void tr_compile(VM, TrCompiler *compiler, char *code, int trace);
void *TrParserAlloc(void *(*)(size_t));
void TrParser(void *, int, OBJ, TrCompiler *);
void TrParserFree(void *, void (*)(void*));
void TrParserTrace(FILE *stream, char *zPrefix);

#endif /* _TINYRB_H_ */
