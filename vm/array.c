#include "tinyrb.h"

tr_array *tr_array_struct(VM)
{
  tr_array *a = (tr_array *) tr_malloc(sizeof(tr_array));
  
  a->type  = TR_ARRAY;
  a->count = 0;
  a->first = NULL;
  a->last  = NULL;

  return a;
}

OBJ tr_array_new(VM)
{
  tr_array *a = tr_array_struct(vm);
  
  tr_obj_init(vm, TR_ARRAY, (OBJ) a, tr_const_get(vm, "Array"));

  return (OBJ) a;
}

static tr_array_entry *tr_array_entry_new(VM, OBJ value)
{
  tr_array_entry *e = (tr_array_entry *) tr_malloc(sizeof(tr_array_entry));
  e->value = value;
  e->next  = NULL;
  e->prev  = NULL;
  return e;
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

OBJ tr_array_push(VM, OBJ self, OBJ item)
{
  tr_array       *a = TR_CARRAY(self);
  tr_array_entry *e = tr_array_entry_new(vm, item);
  
  a->count++;
  
  if (a->first == NULL) {
    a->first = a->last = e;
    return item;
  }
  
  a->last->next = e;
  e->prev = a->last;
  a->last = e;
  
  return item;
}

OBJ tr_array_pop(VM, OBJ self)
{
  tr_array       *a = TR_CARRAY(self);
  tr_array_entry *e;
  OBJ             obj;
  
  if (a->first == NULL)
    return TR_NIL;
  
  e   = a->last;
  obj = e->value;
  a->count--;
  
  a->last = e->prev;
  tr_free(e);
  
  if (a->count == 0)
    a->first = a->last = NULL;
  
  return obj;
}

OBJ tr_array_last(VM, OBJ self)
{
  tr_array *a = TR_CARRAY(self);
  
  if (a->first == NULL)
    return TR_NIL;
  
  return a->last->value;
}

OBJ tr_array_count(VM, OBJ self)
{
  return tr_fixnum_new(vm, TR_CARRAY(self)->count);
}

static tr_array_entry *tr_array_entry_at(VM, OBJ self, int i)
{
  tr_array       *a = TR_CARRAY(self);
  tr_array_entry *e = a->first;
  off_t           c = 0;
  
  if (i >= a->count || i < 0)
    return NULL;
  
  for (c = 0; c < i && e != NULL; ++c)
    e = e->next;
  
  if (c == i && e != NULL)
    return e;
  return NULL;
}

OBJ tr_array_at(VM, OBJ self, int i)
{
  tr_array_entry *e = tr_array_entry_at(vm, self, i);
  
  if (e == NULL)
    return TR_NIL;
  
  return e->value;
}

static OBJ tr_array_at2(VM, OBJ self, OBJ i)
{
  return tr_array_at(vm, self, TR_FIX(i));
}

OBJ tr_array_set(VM, OBJ self, int i, OBJ item)
{
  tr_array_entry *e = tr_array_entry_at(vm, self, i);
  
  if (e == NULL)
    return tr_array_insert(vm, self, i, item);
  
  e->value = item;
  
  return item;
}

OBJ tr_array_set2(VM, OBJ self, OBJ i, OBJ item)
{
  return tr_array_set(vm, self, TR_FIX(i), item);
}

OBJ tr_array_insert(VM, OBJ self, int i, OBJ item)
{
  tr_array       *a = TR_CARRAY(self);
  tr_array_entry *e = tr_array_entry_at(vm, self, i);
  tr_array_entry *n = tr_array_entry_new(vm, item);
  
  if (e == NULL)
    return tr_array_push(vm, self, item);
  
  if (e->prev == NULL)
    a->first = n;
  else
    e->prev->next = n;
  n->prev = e->prev;
  e->prev = n;
  n->next = e;
  
  a->count++;
  
  return item;
}

static OBJ tr_array_insert2(VM, OBJ self, int i, OBJ item)
{
  return tr_array_insert(vm, self, TR_FIX(i), item);
}

void tr_array_init(VM)
{
  OBJ class = tr_class_new(vm, "Array", tr_const_get(vm, "Object"));
  
  tr_def(vm, class, "[]", tr_array_at2, 1);
  tr_def(vm, class, "[]=", tr_array_set2, 2);
  tr_def(vm, class, "last", tr_array_last, 0);
  tr_def(vm, class, "count", tr_array_count, 0);
  tr_def(vm, class, "size", tr_array_count, 0);
  tr_def(vm, class, "<<", tr_array_push, 1);
  tr_def(vm, class, "pop", tr_array_pop, 0);
  tr_def(vm, class, "insert", tr_array_insert2, 2);
}

