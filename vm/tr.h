#ifndef _TINYRB_H_
#define _TINYRB_H_

#include <stdlib.h>
#include <limits.h>
#include "kvec.h"
#include "config.h"

#define TR_ALLOC(T)          (T *)malloc(sizeof(T))
#define TR_ALLOC_N(T,N)      (T *)malloc(sizeof(T)*(N))

#define TR_MEMZERO(X,T)      memset((X), 0, sizeof(T))
#define TR_MEMZERO_N(X,T,N)  memset((X), 0, sizeof(T)*(N))
#define TR_MEMCPY(X,Y,T)     memcpy((X), (Y), sizeof(T))
#define TR_MEMCPY_N(X,Y,T,N) memcpy((X), (Y), sizeof(T)*(N))

#define VM                   TrVM *vm
#define FRAME                &vm->frames[vm->cf]
#define TR_OBJECT_HEADER     

typedef unsigned long OBJ;

enum TR_T {
  TR_T_OBJECT, TR_T_CLASS, TR_T_SYMBOL, TR_T_STRING,
  TR_T_MAX /* keep last */
};

typedef struct {
  kvec_t(OBJ) stack;
  char *fn;
  int line; /* cur line num */
  OBJ self;
} TrFrame;

typedef struct {
  size_t cf; /* current frame */
  TrFrame frames[TR_MAX_FRAME];
} TrVM;

typedef union {
  unsigned char i;
  struct { unsigned char i,a,b,c; } regs;
  struct { char val[4]; } string;
  struct { float val; } number;
} TrOp;

typedef struct {
  TR_OBJECT_HEADER;
} TrObject;

OBJ tr_run(VM, TrOp *code);

#endif /* _TINYRB_H_ */
