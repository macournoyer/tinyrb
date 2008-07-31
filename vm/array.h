#ifndef _ARRAY_H_
#define _ARRAY_H_

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>

typedef struct array_s array_t;

struct array_s {
  size_t  size;
  uint    nalloc;
  uint    nitems;
  void   *items;
};

/* Create an dynamic array of initialy +num+ items */
array_t * array_create(uint num, size_t size);

/* Push a new item to the array returning its ref.
 * If the array is not big enought its size is doubled. */
void * array_push(array_t *a);

/* Destroy the array and frees the memory */
void array_destroy(array_t *a);

#endif /* _ARRAY_H_ */
