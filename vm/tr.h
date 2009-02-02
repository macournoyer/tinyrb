#ifndef _TINYRB_H_
#define _TINYRB_H_

#include <stdlib.h>
#include <limits.h>
#include "kvec.h"
#include "khash.h"
#include "config.h"

#define TR_ALLOC(T)          (T *)malloc(sizeof(T))
#define TR_ALLOC_N(T,N)      (T *)malloc(sizeof(T)*(N))

#define TR_MEMZERO(X,T)      memset((X), 0, sizeof(T))
#define TR_MEMZERO_N(X,T,N)  memset((X), 0, sizeof(T)*(N))
#define TR_MEMCPY(X,Y,T)     memcpy((X), (Y), sizeof(T))
#define TR_MEMCPY_N(X,Y,T,N) memcpy((X), (Y), sizeof(T)*(N))

#define VM                   TrVM *vm
#define FRAME                &vm->frames[vm->cf]
#define TR_OBJECT_HEADER     TR_T type

#define TR_NIL               ((OBJ)0)

typedef unsigned long OBJ;

KHASH_MAP_INIT_STR(str, OBJ);

typedef enum {
  TR_T_Object, TR_T_Class, TR_T_Symbol, TR_T_String,
  TR_T_MAX /* keep last */
} TR_T;

typedef struct {
  TR_OBJECT_HEADER;
} TrObject;

typedef struct {
  TR_OBJECT_HEADER;
  char *ptr;
  size_t len;
} TrString;

typedef struct {
  kvec_t(OBJ) stack;
  char *fn;
  int line; /* cur line num */
  OBJ self;
} TrFrame;

typedef struct {
  size_t cf; /* current frame */
  TrFrame frames[TR_MAX_FRAME];
  khash_t(str) *symbols;
} TrVM;

typedef struct {
  unsigned char i:8;
  int a:12;
  int b:12;
} TrOp;

typedef struct {
  OBJ *k;
  TrOp *code;
  int kn;
  int coden;
} TrBlock;

/* vm */
TrVM *TrVM_new();
void TrVM_destroy(TrVM *vm);
OBJ tr_run(VM, TrBlock *code);

/* string */
OBJ TrSymbol_new(VM, const char *str);

#endif /* _TINYRB_H_ */
