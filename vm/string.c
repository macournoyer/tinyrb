#include "tinyrb.h"

OBJ tr_string_new(VM, const char *ptr)
{
  tr_string *str = (tr_string *) tr_malloc(sizeof(tr_string));
  
  /* tr_obj_init(TR_STRING, (OBJ) str, tr_const_get(vm, "String")); */
  str->len  = strlen(ptr);
  str->ptr  = tr_malloc(str->len * sizeof(char));
  strcpy(str->ptr, ptr);
  
  return (OBJ) str;
}

OBJ tr_intern(VM, const char *ptr)
{
  /* TODO */
  return tr_string_new(vm, ptr);
}

OBJ tr_string_init(VM)
{
  tr_class_new(vm, "String", tr_const_get(vm, "Object"));
}