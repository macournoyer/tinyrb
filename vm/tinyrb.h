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
#define TR_FALSE ((OBJ) 0)
#define TR_TRUE  ((OBJ) 2)
#define TR_NIL   ((OBJ) 4)
#define TR_UNDEF ((OBJ) 6)

#define TR_ERROR -1
#define TR_OK     0

/* conversion */
#define TR_TYPE(o)      (((tr_obj *) o)->type)
#define TR_CTYPE(o,e,t) (assert(TR_TYPE(o) == e), ((t *) o))
#define TR_COBJ(o)      ((tr_obj *) o)
#define TR_CSTRING(o)   TR_CTYPE(o, TR_STRING, tr_string)
#define TR_CFIXNUM(o)   TR_CTYPE(o, TR_FIXNUM, tr_fixnum)
#define TR_CARRAY(o)    TR_CTYPE(o, TR_ARRAY, tr_array)
#define TR_CHASH(o)     TR_CTYPE(o, TR_HASH, tr_hash)
#define TR_CCLASS(o)    TR_CTYPE(o, TR_CLASS, tr_class)
#define TR_CMETHOD(o)   TR_CTYPE(o, TR_METHOD, tr_method)

/* shortcuts */
#define TR_STR(s)       (TR_CSTRING(s)->ptr)
#define TR_FIX(n)       (TR_CFIXNUM(n)->val)
#define VM              tr_vm *vm
#define CUR_FRAME       (&vm->frames[vm->cf])

/* mem stuff */
#define tr_malloc(s)    malloc(s)
#define tr_realloc(c,s) realloc(c,s)
#define tr_free(p)      free((void *) (p))

#define tr_log(m,...)   fprintf(stderr, m "\n", __VA_ARGS__)
#ifdef DEBUG
#define tr_debug(m,...) fprintf(stderr, m "\n", __VA_ARGS__)
#else
#define tr_debug(m,...)
#endif

/* objects */
typedef unsigned long OBJ;
typedef enum { TR_STRING = 0, TR_FIXNUM, TR_HASH, TR_ARRAY, TR_MODULE, TR_CLASS, TR_METHOD, TR_OBJECT } tr_type;

/* TODO puts specific type instead of OBJ??? */

#define ACTS_AS_TR_OBJ /* lol! */ \
  tr_type          type;  \
  OBJ              ivars; \
  struct tr_class *class; \
  struct tr_class *metaclass

typedef struct tr_class {
  ACTS_AS_TR_OBJ;
  OBJ              name;
  OBJ              methods;
  struct tr_class *super;
} tr_class;

typedef struct tr_obj {
  ACTS_AS_TR_OBJ;
} tr_obj;

typedef struct tr_string {
  ACTS_AS_TR_OBJ;
  char    *ptr;
  size_t   len;
} tr_string;

typedef struct tr_fixnum {
  ACTS_AS_TR_OBJ;
  int   val;
} tr_fixnum;

typedef struct tr_array {
  ACTS_AS_TR_OBJ;
  size_t   count;
  uint     nalloc;
  OBJ     *items;
} tr_array;

typedef struct tr_hash_entry {
  OBJ     k, v;
  uint    h;
  struct  tr_hash_entry *next;
} tr_hash_entry;

typedef struct tr_hash {
  ACTS_AS_TR_OBJ;
  uint            tablelength;
  tr_hash_entry **table;
  uint            hash_entrycount;
  uint            loadlimit;
  uint            primeindex;
} tr_hash;

typedef struct tr_method {
  ACTS_AS_TR_OBJ;
  OBJ      name;
  OBJ    (*func)();
  int      argc;
} tr_method;

/* vm structs */
typedef struct tr_op {
  tr_inst_e inst;
  void     *cmd[5];
} tr_op;

typedef struct tr_frame {
  OBJ  stack;
  OBJ  consts;
  OBJ  locals;
} tr_frame;

typedef struct tr_vm {
  off_t    cf; /* current frame */
  tr_frame frames[TR_MAX_FRAMES];
} tr_vm;

/* vm */
void tr_init(tr_vm *vm);
int tr_run(tr_vm *vm, tr_op *ops, size_t n);

/* class */
void tr_const_set(VM, const char *name, OBJ obj);
OBJ tr_const_get(VM, const char *name);
OBJ tr_send(OBJ obj, OBJ message, int argc, OBJ argv[]);
OBJ tr_def(OBJ obj, const char *name, OBJ (*func)(), int argc);
OBJ tr_metadef(OBJ obj, const char *name, OBJ (*func)(), int argc);
OBJ tr_class_new(VM, const char* name, OBJ super);
OBJ tr_new(OBJ class);

/* string */
OBJ tr_string_new(const char *ptr);
OBJ tr_intern(const char *ptr);

/* fixnum */
OBJ tr_fixnum_new(int val);

/* hash */
OBJ tr_hash_new();
OBJ tr_hash_set(OBJ h, OBJ k, OBJ v);
OBJ tr_hash_get(OBJ h, OBJ k);
size_t tr_hash_count(OBJ h);
OBJ tr_hash_delete(OBJ h, OBJ k);
OBJ tr_hash_clear(OBJ h);

/* array */
OBJ tr_array_new();
void tr_array_push(OBJ a, OBJ item);
OBJ tr_array_pop(OBJ a);
size_t tr_array_count(OBJ a);
void tr_array_destroy(OBJ a);

#endif /* _TINYRB_H_ */
