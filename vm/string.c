#include "tinyrb.h"

OBJ tr_string_new(const char *ptr)
{
  tr_string *str = tr_malloc(sizeof(tr_string));
  
  str->type = TR_STRING;
  str->len  = strlen(ptr);
  str->ptr  = tr_malloc(str->len * sizeof(char));
  strcpy(str->ptr, ptr);
  
  return (OBJ) str;
}

OBJ tr_intern(const char *ptr)
{
  /* TODO */
  return tr_string_new(ptr);
}