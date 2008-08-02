#ifndef _TINYRB_H_
#define _TINYRB_H_

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "inst.h"

#define TR_ERROR -1
#define TR_OK     0

typedef unsigned long OBJECT;

typedef struct tr_array {
  size_t  size;
  uint    nalloc;
  uint    nitems;
  void   *items;
} tr_array;

typedef struct tr_hash_entry {
  void *k, *v;
  unsigned int h;
  struct tr_hash_entry *next;
} tr_hash_entry;

typedef struct tr_hash {
  unsigned int tablelength;
  tr_hash_entry **table;
  unsigned int hash_entrycount;
  unsigned int loadlimit;
  unsigned int primeindex;
  unsigned int (*hashfn) (void *k);
  int (*eqfn) (void *k1, void *k2);
} tr_hash;


/* instruction */
typedef struct tr_inst {
  tr_inst_e code;
  OBJECT    ops[5];
} tr_inst ;

/* stack frame */
typedef struct tr_sf {
  off_t    sp;
  tr_array *stack;
} tr_sf;

int tr_exec_inst(tr_sf *sf, tr_inst *inst);
int tr_exec_insts(tr_inst *insts, size_t n);

#endif /* _TINYRB_H_ */
