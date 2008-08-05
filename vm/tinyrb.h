#ifndef _TINYRB_H_
#define _TINYRB_H_

#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "inst.h"

#define TR_MAX_FRAMES 250

/* special magic consts */
#define TR_FALSE (OBJ) 0
#define TR_TRUE  (OBJ) 2
#define TR_NIL   (OBJ) 4
#define TR_UNDEF (OBJ) 6

#define TR_ERROR -1
#define TR_OK     0

/* conversion */
#define TR_TYPE(o)      (((tr_obj *) o)->type)
#define TR_CTYPE(o,e,t) (assert(TR_TYPE(o) == e), ((t *) o))
#define TR_CSTRING(o)   TR_CTYPE(o, TR_STRING, tr_string)
#define TR_CARRAY(o)    TR_CTYPE(o, TR_ARRAY, tr_array)
#define TR_CHASH(o)     TR_CTYPE(o, TR_HASH, tr_hash)

/* shortcuts */
#define TR_STR(s)       (TR_CSTRING(s)->ptr)
#define VM              tr_vm *vm
#define CUR_FRAME       (&vm->frames[vm->cf])

/* mem stuff */
#define tr_malloc(s)    malloc(s)
#define tr_realloc(c,s) realloc(c,s)
#define tr_free(p)      free(p)

#define tr_log(m,...)   fprintf(stderr, m, __VA_ARGS__)

typedef unsigned long OBJ;

/* objects */
typedef enum { TR_STRING, TR_HASH, TR_ARRAY, TR_MODULE, TR_CLASS } tr_type;

typedef struct tr_obj {
  tr_type type;
} tr_obj;

typedef struct tr_string {
  tr_type  type;
  char    *ptr;
  size_t   len;
} tr_string;

typedef struct tr_array {
  tr_type  type;
  size_t   count;
  uint     nalloc;
  OBJ     *items;
} tr_array;

typedef struct tr_hash_entry {
  void   *k, *v;
  uint    h;
  struct  tr_hash_entry *next;
} tr_hash_entry;

typedef struct tr_hash {
  tr_type         type;
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

/* vm structs */
typedef struct tr_op {
  tr_inst_e inst;
  OBJ       cmd[5];
} tr_op;

typedef struct tr_frame {
  OBJ       stack;
  tr_hash  *consts;
} tr_frame;

typedef struct tr_vm {
  off_t    cf; /* current frame */
  tr_frame frames[TR_MAX_FRAMES];
} tr_vm;

/* vm */
void tr_init(tr_vm *vm);
int tr_run(tr_vm *vm, tr_op *ops, size_t n);

/* string */
OBJ tr_string_new(const char *ptr);
OBJ tr_intern(const char *ptr);

/* hash */
tr_hash *tr_hash_new();
int tr_hash_set(tr_hash *h, void *k, void *v);
void *tr_hash_get(tr_hash *h, void *k);

/* array */
OBJ tr_array_new();
void tr_array_push(OBJ a, OBJ item);
OBJ tr_array_pop(OBJ a);
size_t tr_array_count(OBJ a);
void tr_array_destroy(OBJ a);

/* module */
OBJ tr_send(VM, OBJ obj, OBJ message, int argc, OBJ argv[]);
void tr_def(VM, OBJ mod, const char *name, OBJ (*func)(), int argc);
OBJ tr_module_new(VM, const char *name);
void tr_builtins_add(VM);

#endif /* _TINYRB_H_ */
