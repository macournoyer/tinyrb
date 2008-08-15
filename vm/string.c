#include "tinyrb.h"

OBJ tr_string_new(VM, const char *ptr)
{
  return tr_string_new2(vm, ptr, strlen(ptr));
}

OBJ tr_string_new2(VM, const char *ptr, size_t len)
{
  tr_string *str = (tr_string *) tr_malloc(sizeof(tr_string));
  
  tr_obj_init(vm, TR_STRING, (OBJ) str, tr_const_get(vm, "String"));
  str->len  = len;
  str->ptr  = tr_malloc(str->len * sizeof(char));
  str->interned = 0;
  strcpy(str->ptr, ptr);
  
  return (OBJ) str;
}

OBJ tr_string_concat(VM, OBJ self, OBJ str2)
{
  tr_string *str = (tr_string *) tr_string_new(vm, TR_STR(self));
  
  str->len += TR_CSTRING(str2)->len;
  str->ptr  = tr_realloc(str->ptr, str->len);
  strcat(str->ptr, TR_STR(str2));
  
  return (OBJ) str;
}

static OBJ tr_string_self(VM, OBJ self)
{
  return self;
}

static OBJ tr_string_size(VM, OBJ self)
{
  return tr_fixnum_new(vm, TR_CSTRING(self)->len);
}

static OBJ tr_string_slice(VM, OBJ self, OBJ start, OBJ len)
{
  int        s = TR_FIX(start), l = TR_FIX(len);
  tr_string *str   = TR_CSTRING(self);
  OBJ        slice = tr_string_new2(vm, "", l);
  
  if (s > str->len)
    return TR_NIL;
  
  if (s + l > str->len)
    l = str->len - s;
  
  memcpy(TR_STR(slice), str->ptr + s, l);
  
  return slice;
}

static OBJ tr_string_eq(VM, OBJ self, OBJ other)
{
  char *s1 = TR_STR(self), *s2 = TR_STR(other);
  return TR_CBOOL(strcmp(s1, s2) == 0);
}

void tr_string_init(VM)
{
  OBJ class = tr_class_new(vm, "String", tr_const_get(vm, "Object"));
  
  tr_def(vm, class, "+", tr_string_concat, 1);
  tr_def(vm, class, "to_s", tr_string_self, 0);
  tr_def(vm, class, "size", tr_string_size, 0);
  tr_def(vm, class, "[]", tr_string_slice, 2);
  tr_def(vm, class, "==", tr_string_slice, 1);
}

/* symbol */

#define INDEX2SYM(i) (OBJ)((i) << TR_SPECIAL_SHIFT | TR_SYMBOL_FLAG)
#define SYM2INDEX(i) (int)((i) >> TR_SPECIAL_SHIFT)

OBJ tr_symbol_get(VM, OBJ obj)
{
  int i          = SYM2INDEX(obj);
  tr_string *str = (tr_string *) tr_array_at(vm, (OBJ) vm->symbols, i);
  
  assert(TR_TYPE(obj) == TR_SYMBOL);
  assert((OBJ) str != TR_NIL);
  
  if (!str->interned) {
    tr_obj_init(vm, TR_STRING, (OBJ) str, tr_const_get(vm, "Symbol"));
    str->interned = 1;
  }
  
  return (OBJ) str;
}

OBJ tr_intern(VM, char *ptr)
{
  tr_array_entry *e = vm->symbols->first;
  tr_string      *str;
  OBJ             i = 0;
  
  while (e != NULL) {
    str = (tr_string *) e->value;
    if (strcmp(ptr, str->ptr) == 0)
      return INDEX2SYM(i);
    i++;
    e = e->next;
  }
  
  /* new symbol */
  str = (tr_string *) tr_malloc(sizeof(tr_string));
  str->type = TR_STRING;
  str->len  = strlen(ptr);
  str->ptr  = tr_malloc(str->len * sizeof(char));
  str->interned = 0;
  strcpy(str->ptr, ptr);
  tr_array_push(vm, (OBJ) vm->symbols, (OBJ) str);
  
  return INDEX2SYM(i);
}

void tr_symbol_init(VM)
{
  OBJ class = tr_class_new(vm, "Symbol", tr_const_get(vm, "String"));
}
