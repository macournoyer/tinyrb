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

#define TR_FALSE (OBJ) 0
#define TR_TRUE  (OBJ) 2
#define TR_NIL   (OBJ) 4
#define TR_UNDEF (OBJ) 6

#define TR_ERROR -1
#define TR_OK     0

#define VM            tr_vm *vm
#define CUR_FRAME     (&vm->frames[vm->cf])
#define tr_malloc(s)  malloc(s)
#define tr_free(p)    free(p)
#define tr_log(m,...) fprintf(stderr, m, __VA_ARGS__)

typedef unsigned long OBJ;
typedef char *        SYM;

/* objects */
typedef enum { TR_HASH, TR_ARRAY, TR_MODULE, TR_CLASS } tr_type;

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

typedef struct tr_module {
  tr_type  type;
  char    *name;
  tr_hash *methods;
  tr_hash *imethods;
} tr_module;

typedef struct tr_method {
  char    *name;
  OBJ    (*func)();
  int      argc;
} tr_method;

typedef union tr_obj {
  tr_type type;
  /* TODO union or struct cast? */
} tr_obj;

/* vm structs */
typedef struct tr_op {
  tr_inst_e inst;
  OBJ       cmd[5];
} tr_op;

typedef struct tr_frame {
  tr_array *stack;
  tr_hash  *consts;
} tr_frame;

typedef struct tr_vm {
  off_t    cf; /* current frame */
  tr_frame frames[TR_MAX_FRAMES];
} tr_vm;

/* vm */
void tr_init(tr_vm *vm);
int tr_run(tr_vm *vm, tr_op *ops, size_t n);

/* hash */
tr_hash *tr_hash_new();
int tr_hash_set(tr_hash *h, void *k, void *v);
void *tr_hash_get(tr_hash *h, void *k);

/* array */
tr_array *tr_array_new(uint num, size_t size);
void *tr_array_push(tr_array *a);
void *tr_array_pop(tr_array *a);
void tr_array_destroy(tr_array *a);

/* module */
OBJ tr_call(VM, OBJ obj, const char *method, int argc, OBJ argv[]);
void tr_def(VM, OBJ mod, const char *name, OBJ (*func)(), int argc);
OBJ tr_module_new(VM, const char *name);
void tr_builtins_add(VM);

#endif /* _TINYRB_H_ */
