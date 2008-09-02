#include "tinyrb.h"

OBJ tr_string_new(VM, char *ptr)
{
  return tr_string_new2(vm, ptr, strlen(ptr));
}

OBJ tr_string_new2(VM, char *ptr, size_t len)
{
  tr_string *str = (tr_string *) tr_malloc(sizeof(tr_string));
  
  tr_obj_init(vm, TR_STRING, (OBJ) str, tr_const_get(vm, "String"));
  str->len    = len;
  str->ptr    = tr_malloc(str->len * sizeof(char));
  str->symbol = TR_NIL;
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

static OBJ tr_string_size(VM, OBJ self)
{
  return tr_fixnum_new(vm, TR_CSTRING(self)->len);
}

static OBJ tr_string_substring(VM, OBJ self, OBJ start, OBJ len)
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

static OBJ tr_string_cmp(VM, OBJ self, OBJ other)
{
  if (TR_TYPE(other) != TR_STRING)
    return tr_fixnum_new(vm, -1);
  
  char *s1 = TR_STR(self), *s2 = TR_STR(other);
  
  return tr_fixnum_new(vm, strcmp(s1, s2));
}

static OBJ tr_string_replace(VM, OBJ self, OBJ other)
{
  tr_string *s = TR_CSTRING(self);
  tr_string *o = TR_CSTRING(other);
  
  s->ptr = o->ptr;
  s->len = o->len;
  
  return other;
}

static OBJ tr_string_to_sym(VM, OBJ self)
{
  return tr_intern(vm, TR_CSTRING(self)->ptr);
}

OBJ tr_string_dup(VM, OBJ self)
{
  tr_string *s = TR_CSTRING(self);
  return tr_string_new2(vm, s->ptr, s->len);
}

void tr_string_init(VM)
{
  OBJ class = tr_class_new(vm, "String", tr_const_get(vm, "Object"));
  
  tr_def(vm, class, "+", tr_string_concat, 1);
  tr_def(vm, class, "size", tr_string_size, 0);
  tr_def(vm, class, "substring", tr_string_substring, 2);
  tr_def(vm, class, "<=>", tr_string_cmp, 1);
  tr_def(vm, class, "replace", tr_string_replace, 1);
  tr_def(vm, class, "to_sym", tr_string_to_sym, 0);
  tr_def(vm, class, "dup", tr_string_dup, 0);
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
  
  if (str->class == NULL)
    tr_obj_init(vm, TR_STRING, (OBJ) str, tr_const_get(vm, "Symbol"));
  
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
  str->type   = TR_STRING;
  str->class  = NULL;
  str->len    = strlen(ptr);
  str->ptr    = tr_malloc(str->len * sizeof(char));
  str->symbol = INDEX2SYM(i);
  strcpy(str->ptr, ptr);
  tr_array_push(vm, (OBJ) vm->symbols, (OBJ) str);
  
  return str->symbol;
}

static OBJ tr_symbol_eq(VM, OBJ self, OBJ other)
{
  OBJ other_sym = TR_TYPE(other) == TR_STRING ? TR_CSTRING(other)->symbol : other;  
  return TR_CBOOL(TR_TYPE(other) == TR_SYMBOL && TR_CSTRING(self)->symbol == other);
}

void tr_symbol_init(VM)
{
  OBJ class = tr_class_new(vm, "Symbol", tr_const_get(vm, "String"));
  tr_def(vm, class, "==", tr_symbol_eq, 1);
}
