#include "tinyrb.h"

#define TR_ARRAY_N    1024 /* items in new array, TODO lower? */
#define TR_ARRAY_SIZE sizeof(OBJ *)
#define OBJ_AT(a,i)   *((OBJ *) (a)->items + TR_ARRAY_SIZE * (i))

OBJ tr_array_new(VM)
{
  tr_array *a = (tr_array *) tr_malloc(sizeof(tr_array));
  if (a == NULL)
    return TR_NIL;
  
  tr_obj_init(vm, TR_ARRAY, (OBJ) a, tr_const_get(vm, "Array"));
  a->nalloc = TR_ARRAY_N;
  a->count  = 0;
  a->items  = tr_malloc(TR_ARRAY_N * TR_ARRAY_SIZE);
  
  if (a->items == NULL) {
    tr_free(a);
    return TR_NIL;
  }

  return (OBJ) a;
}

OBJ tr_array_create(VM, int argc, ...)
{
  OBJ     a = tr_array_new(vm);
  OBJ     o;
  va_list argp;
  size_t  i;
  
  va_start(argp, argc);
  
  for (i = 0; i < argc; ++i)
    tr_array_push(vm, a, va_arg(argp, OBJ));
  
  va_end(argp);
  
  return a;
}

void tr_array_push(VM, OBJ self, OBJ item)
{
  tr_array *a = TR_CARRAY(self);
  OBJ      *slot;
  
  if (a->count == a->nalloc) {
    /* array is full, double the size */

    void   *new;
    size_t  size;
    
    size = TR_ARRAY_SIZE * a->nalloc;

    new = tr_realloc(a->items, 2 * size);
    assert(new);
    
    a->items = new;
    a->nalloc *= 2;
  }
  
  slot = (OBJ *) a->items + TR_ARRAY_SIZE * a->count;
  memcpy((void *) slot, &item, sizeof(OBJ *));
  a->count++;
}

OBJ tr_array_pop(VM, OBJ self)
{
  tr_array *a   = TR_CARRAY(self);
  OBJ       obj = tr_array_last(vm, self);

  if (a->count > 0)
    TR_CARRAY(a)->count--;

  return obj;
}

OBJ tr_array_last(VM, OBJ self)
{
  tr_array *a = TR_CARRAY(self);
  
  if (a->count == 0)
    return TR_NIL;
  
  return OBJ_AT(a, a->count - 1);
}

OBJ tr_array_count(VM, OBJ self)
{
  tr_array *a = TR_CARRAY(self);
  
  return tr_fixnum_new(vm, a->count);
}

void tr_array_destroy(VM, OBJ self)
{
  tr_array *a = TR_CARRAY(self);
  
  tr_free(a->items);
  tr_free(a);
}

OBJ tr_array_at(VM, OBJ self, int i)
{
  tr_array *a = TR_CARRAY(self);
  
  if (i >= a->count)
    return TR_NIL;
  
  return OBJ_AT(a, i);
}

static OBJ tr_array_at2(VM, OBJ self, OBJ i)
{
  return tr_array_at(vm, self, TR_FIX(i));
}

void tr_array_init(VM)
{
  OBJ class = tr_class_new(vm, "Array", tr_const_get(vm, "Object"));
  
  tr_def(vm, class, "[]", tr_array_at2, 1);
  tr_def(vm, class, "last", tr_array_last, 0);
  tr_def(vm, class, "count", tr_array_count, 0);
  tr_def(vm, class, "size", tr_array_count, 0);
}

