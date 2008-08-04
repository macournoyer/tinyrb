#include "tinyrb.h"

#define TR_ARRAY_N    5 /* items in new array */
#define TR_ARRAY_SIZE sizeof(OBJ)

tr_array * tr_array_new()
{
  tr_array *a = (tr_array *) tr_malloc(sizeof(tr_array));
  if (a == NULL)
    return NULL;
  
  a->nalloc = TR_ARRAY_N;
  a->count  = 0;
  a->items  = tr_malloc(TR_ARRAY_N * TR_ARRAY_SIZE);
  
  if (a->items == NULL) {
    tr_free(a);
    return NULL;
  }

  return a;
}

void *tr_array_push(tr_array *a)
{
  void *item;
  
  if (a->count == a->nalloc) {
    /* array is full, double the size */

    void   *new;
    size_t  size;
    
    size = TR_ARRAY_SIZE * a->nalloc;

    new = tr_realloc(a->items, 2 * size);
    if (new == NULL)
      return NULL;
    
    a->items = new;
    a->nalloc *= 2;
  }
  
  item = (u_char *) a->items + TR_ARRAY_SIZE * a->count;
  a->count++;
  
  return item;
}

void *tr_array_pop(tr_array *a)
{
  if (a->count == 0)
    return NULL;
  
  return (void *) a->items + TR_ARRAY_SIZE * --a->count;
}

size_t tr_array_count(tr_array *a)
{
  return a->count;
}

void tr_array_destroy(tr_array *a)
{
  tr_free(a->items);
  tr_free(a);
}
