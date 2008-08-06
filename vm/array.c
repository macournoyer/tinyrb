#include "tinyrb.h"

#define TR_ARRAY_N    5 /* items in new array */
#define TR_ARRAY_SIZE sizeof(OBJ *)

OBJ tr_array_new(VM)
{
  tr_array *a = (tr_array *) tr_malloc(sizeof(tr_array));
  if (a == NULL)
    return TR_NIL;
  
  a->type   = TR_ARRAY;
  a->nalloc = TR_ARRAY_N;
  a->count  = 0;
  a->items  = tr_malloc(TR_ARRAY_N * TR_ARRAY_SIZE);
  
  if (a->items == NULL) {
    tr_free(a);
    return TR_NIL;
  }

  return (OBJ) a;
}

void tr_array_push(VM, OBJ o, OBJ item)
{
  tr_array *a = TR_CARRAY(o);
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

OBJ tr_array_pop(VM, OBJ o)
{
  tr_array *a = TR_CARRAY(o);
  
  if (a->count == 0)
    return TR_NIL;
  
  return *((OBJ *) a->items + TR_ARRAY_SIZE * --a->count);
}

size_t tr_array_count(VM, OBJ o)
{
  tr_array *a = TR_CARRAY(o);
  
  return a->count;
}

void tr_array_destroy(VM, OBJ o)
{
  tr_array *a = TR_CARRAY(o);
  
  tr_free(a->items);
  tr_free(a);
}
