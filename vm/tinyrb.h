#ifndef _TINYRB_H_
#define _TINYRB_H_

#include <assert.h>
#include <fcntl.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "inst.h"

#define TR_MAX_FRAMES 250

/* special magic consts */
#define TR_FALSE ((OBJ) 0)
#define TR_TRUE  ((OBJ) 2)
#define TR_NIL   ((OBJ) 4)
#define TR_UNDEF ((OBJ) 6)

/* flags */
#define TR_SPECIAL_SHIFT 8
#define TR_SYMBOL_FLAG   0x0e
#define TR_SPECIAL(o,f)  (((o)&(f))==(f))

/* conversion */
#define TR_TYPE(o)       tr_type_get((OBJ) o)
#define TR_CTYPE(o,e,t)  (TR_ASSERT(TR_TYPE(o) == e, "unexpected type: %d for %d", TR_TYPE(o), e), ((t *) o))
#define TR_COBJ(o)       (TR_ASSERT(TR_TYPE(o) < TR_SPECIAL, "not an object"), (tr_obj *) o)
#define TR_CSTRING(o)    TR_CTYPE(o, TR_STRING, tr_string)
#define TR_CFIXNUM(o)    TR_CTYPE(o, TR_FIXNUM, tr_fixnum)
#define TR_CARRAY(o)     TR_CTYPE(o, TR_ARRAY, tr_array)
#define TR_CHASH(o)      TR_CTYPE(o, TR_HASH, tr_hash)
#define TR_CCLASS(o)     TR_CTYPE(o, TR_CLASS, tr_class)
#define TR_CMETHOD(o)    TR_CTYPE(o, TR_METHOD, tr_method)
#define TR_CPROC(o)      TR_CTYPE(o, TR_PROC, tr_proc)
#define TR_CRANGE(o)     TR_CTYPE(o, TR_RANGE, tr_range)
#define TR_CIO(o)        TR_CTYPE(o, TR_IO, tr_io)
#define TR_CBOOL(o)      ((o)?TR_TRUE:TR_FALSE);
#define TR_CSYMBOL(o)    (TR_ASSERT(TR_TYPE(o)==TR_SYMBOL, "not a symbol (%d)", TR_TYPE(o)), (tr_string*) tr_symbol_get(vm, o))

/* shortcuts */
#define TR_STR(s)        (TR_TYPE(s)==TR_STRING ? TR_CSTRING(s)->ptr : TR_CSYMBOL(s)->ptr)
#define TR_FIX(n)        (TR_CFIXNUM(n)->val)
#define TR_SYM(s)        (TR_TYPE(s)==TR_STRING ? tr_intern(vm, TR_CSTRING(s)->ptr) : s)
#define VM               tr_vm *vm
#define CUR_FRAME        (&vm->frames[vm->cf])
#define TR_ASSERT(c,...) ((c) ? NULL : tr_raise(vm, __VA_ARGS__))

enum { CODE = 0, FILENAME, ARGC, LOCALC, LABELS };
#define TR_OPS(p,i) tr_array_at(vm,p,i)
#define TR_2OPS(p)  tr_array_create(vm,5,p->ops,p->filename,tr_fixnum_new(vm,p->argc),tr_fixnum_new(vm,p->localc),p->labels)

/* mem stuff */
#define tr_malloc(s)     malloc(s)
#define tr_realloc(c,s)  realloc(c,s)
#define tr_free(p)       free((void *) (p))

#define tr_log(m,...)    fprintf(stderr, m "\n", __VA_ARGS__)

/* objects */
typedef unsigned long OBJ;
typedef enum {
  TR_STRING = 0, TR_FIXNUM, TR_HASH, TR_ARRAY, TR_MODULE, TR_CLASS,
  TR_IO, TR_METHOD, TR_PROC, TR_RANGE, TR_OBJECT,
  TR_SPECIAL, TR_SYMBOL /* put all "special" types here */
} tr_type;

#define ACTS_AS_TR_OBJ /* lol! */ \
  tr_type          type;  \
  OBJ              ivars; \
  struct tr_class *class; \
  struct tr_class *metaclass; \
  struct tr_array *modules

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
  OBJ      symbol;
} tr_string;

typedef struct tr_fixnum {
  ACTS_AS_TR_OBJ;
  int   val;
} tr_fixnum;

typedef struct tr_array_entry {
  OBJ                    value;
  struct tr_array_entry *next;
  struct tr_array_entry *prev;
} tr_array_entry;

typedef struct tr_array {
  ACTS_AS_TR_OBJ;
  size_t          count;
  tr_array_entry *first;
  tr_array_entry *last;
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

typedef struct tr_io {
  ACTS_AS_TR_OBJ;
  int  fd;
} tr_io;

typedef struct tr_range {
  ACTS_AS_TR_OBJ;
  OBJ  first;
  OBJ  last;
} tr_range;

typedef struct tr_proc {
  ACTS_AS_TR_OBJ;
  off_t     cf;
  OBJ       ops;
  OBJ       filename;
  int       argc;
  int       localc;
  OBJ       labels;
} tr_proc;

typedef struct tr_method {
  ACTS_AS_TR_OBJ;
  OBJ     name;
  OBJ     filename;
  OBJ   (*func)();
  int     argc;
  int     localc;
  OBJ     ops;
  OBJ     labels;
} tr_method;

/* vm structs */
typedef struct tr_frame {
  OBJ  stack;
  OBJ  consts;
  OBJ  locals;
  OBJ  self;
  OBJ  class;
  OBJ  block;
  OBJ  filename;
  uint line; /* cur line num */
} tr_frame;

typedef struct tr_vm {
  off_t     cf; /* current frame */
  tr_frame  frames[TR_MAX_FRAMES];
  OBJ       globals;
  tr_array *symbols;
} tr_vm;

/* vm */
void tr_init(VM, int argc, char *argv[]);
OBJ tr_run(VM, OBJ filename, OBJ ops);
void tr_raise(VM, char *msg, ...);
void tr_next_frame(VM, OBJ obj, OBJ class);
void tr_prev_frame(VM);

/* class */
OBJ tr_def(VM, OBJ obj, char *name, OBJ (*func)(), int argc);
OBJ tr_metadef(VM, OBJ obj, char *name, OBJ (*func)(), int argc);
OBJ tr_alias(VM, OBJ obj, OBJ new_name, OBJ name);
OBJ tr_def_ops(VM, OBJ class, OBJ name, OBJ ops);
OBJ tr_class_new(VM, char* name, OBJ super);
OBJ tr_class_define(VM, OBJ name, OBJ cbase, OBJ super, OBJ ops, int define_type);
OBJ tr_metaclass_new(VM);

/* module */
OBJ tr_module_new(VM, char* name);
OBJ tr_module_include(VM, OBJ self, OBJ module);

/* object */
void tr_const_set(VM, char *name, OBJ obj);
OBJ tr_const_get(VM, char *name);
int tr_const_defined(VM, char *name);
OBJ tr_special_get(VM, OBJ obj);
OBJ tr_send(VM, OBJ obj, OBJ message, int argc, OBJ argv[], OBJ block_ops);
OBJ tr_send2(VM, OBJ obj, char *message, int argc, ...);
tr_type tr_type_get(OBJ obj);
void tr_obj_init(VM, tr_type type, OBJ obj, OBJ class);
OBJ tr_new(VM, OBJ class, int argc, OBJ argv[]);
OBJ tr_new2(VM, OBJ class);

/* proc */
OBJ tr_proc_new(VM, OBJ ops);
OBJ tr_proc_call(VM, OBJ self, int argc, OBJ argv[]);
void tr_proc_init(VM);

/* string */
OBJ tr_string_new(VM, char *ptr);
OBJ tr_string_new2(VM, char *ptr, size_t len);
OBJ tr_intern(VM, char *ptr);
OBJ tr_string_concat(VM, OBJ self, OBJ str2);

/* symbol */
OBJ tr_symbol_get(VM, OBJ obj);

/* fixnum */
OBJ tr_fixnum_new(VM, int val);

/* hash */
OBJ tr_hash_new(VM);
OBJ tr_hash_set(VM, OBJ h, OBJ k, OBJ v);
OBJ tr_hash_get(VM, OBJ h, OBJ k);
OBJ tr_hash_count(VM, OBJ h);
OBJ tr_hash_delete(VM, OBJ h, OBJ k);
OBJ tr_hash_clear(VM, OBJ h);

/* array */
tr_array *tr_array_struct(VM);
OBJ tr_array_new(VM);
OBJ tr_array_create(VM, int argc, ...);
OBJ tr_array_push(VM, OBJ a, OBJ item);
OBJ tr_array_pop(VM, OBJ a);
OBJ tr_array_last(VM, OBJ o);
OBJ tr_array_count(VM, OBJ a);
OBJ tr_array_at(VM, OBJ self, int i);
OBJ tr_array_set(VM, OBJ self, int i, OBJ item);
OBJ tr_array_insert(VM, OBJ self, int i, OBJ item);
void tr_array_init(VM);

/* range */
OBJ tr_range_new(VM, OBJ first, OBJ last);

/* io */
OBJ tr_io_new(VM, int fd);

/* misc init */
void tr_vm_init(VM);
void tr_special_init(VM);
void tr_symbol_init(VM);
void tr_class_init(VM);
void tr_module_init(VM);
void tr_object_init(VM);
void tr_kernel_init(VM);
void tr_string_init(VM);
void tr_fixnum_init(VM);
void tr_range_init(VM);
void tr_io_init(VM);

#endif /* _TINYRB_H_ */
