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
  strcpy(str->ptr, ptr);
  
  return (OBJ) str;
}

OBJ tr_intern(VM, const char *ptr)
{
  /* TODO */
  tr_string *str = (tr_string *) tr_malloc(sizeof(tr_string));
  
  /* tr_obj_init(vm, TR_STRING, (OBJ) str, tr_const_get(vm, "Symbol")); */
  str->len  = strlen(ptr);
  str->ptr  = tr_malloc(str->len * sizeof(char));
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
