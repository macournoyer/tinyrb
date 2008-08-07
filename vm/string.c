#include "tinyrb.h"

OBJ tr_string_new(VM, const char *ptr)
{
  tr_string *str = (tr_string *) tr_malloc(sizeof(tr_string));
  
  tr_obj_init(vm, TR_STRING, (OBJ) str, tr_const_get(vm, "String"));
  str->len  = strlen(ptr);
  str->ptr  = tr_malloc(str->len * sizeof(char));
  strcpy(str->ptr, ptr);
  
  return (OBJ) str;
}

OBJ tr_intern(VM, const char *ptr)
{
  /* TODO */
  tr_string *str = (tr_string *) tr_malloc(sizeof(tr_string));
  
  str->len  = strlen(ptr);
  str->ptr  = tr_malloc(str->len * sizeof(char));
  strcpy(str->ptr, ptr);
  
  return (OBJ) str;
}

static OBJ tr_string_concat(VM, OBJ self, OBJ str2)
{
  OBJ str = tr_string_new(vm, TR_STR(self));
  
  TR_STR(str) = tr_realloc(TR_STR(str), TR_CSTRING(str)->len + TR_CSTRING(str2)->len);
  strcat(TR_STR(str), TR_STR(str2));
  
  return str;
}

static OBJ tr_string_self(VM, OBJ self)
{
  return self;
}

void tr_string_init(VM)
{
  OBJ class = tr_class_new(vm, "String", tr_const_get(vm, "Object"));
  
  tr_def(vm, class, "+", tr_string_concat, 1);
  tr_def(vm, class, "to_s", tr_string_self, 0);
}
