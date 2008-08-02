#ifndef _TINYRB_H_
#define _TINYRB_H_

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "inst.h"

#define TR_MAX_FRAMES 250

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
  void   *k, *v;
  uint    h;
  struct  tr_hash_entry *next;
} tr_hash_entry;

typedef struct tr_hash {
  uint            tablelength;
  tr_hash_entry **table;
  uint            hash_entrycount;
  uint            loadlimit;
  uint            primeindex;
  uint            (*hashfn) (void *k);
  int             (*eqfn) (void *k1, void *k2);
} tr_hash;


typedef struct tr_op {
  tr_inst_e inst;
  OBJECT    cmd[5];
} tr_op;

typedef struct tr_frame {
  tr_array *stack;
  tr_hash  *consts;
} tr_frame;

typedef struct tr_vm {
  off_t    cf; /* current frame */
  tr_frame frames[TR_MAX_FRAMES];
} tr_vm;

void tr_init(tr_vm *vm);
int tr_run(tr_vm *vm, tr_op *ops, size_t n);

tr_hash *tr_hash_new(unsigned int minsize);

tr_array *tr_array_create(uint num, size_t size);
void *tr_array_push(tr_array *a);
void tr_array_destroy(tr_array *a);

#endif /* _TINYRB_H_ */
