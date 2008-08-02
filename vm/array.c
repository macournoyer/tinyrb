#include "tinyrb.h"

tr_array * tr_array_create(uint num, size_t size)
{
  tr_array *a = (tr_array *) malloc(sizeof(tr_array));
  if (a == NULL)
    return NULL;
  
  a->size   = size;
  a->nalloc = num;
  a->nitems = 0;
  a->items  = malloc(num * size);
  
  if (a->items == NULL) {
    free(a);
    return NULL;
  }

  return a;
}

void * tr_array_push(tr_array *a)
{
  void *item;
  
  if (a->nitems == a->nalloc) {
    /* array is full, double the size */

    void   *new;
    size_t  size;
    
    size = a->size * a->nalloc;

    new = realloc(a->items, 2 * size);
    if (new == NULL)
      return NULL;
    
    a->items = new;
    a->nalloc *= 2;
  }
  
  item = (u_char *) a->items + a->size * a->nitems;
  a->nitems++;
  
  return item;
}

void tr_array_destroy(tr_array *a)
{
  void *items;
  
  items = a->items;
  
  free(items);
  free(a);
}
