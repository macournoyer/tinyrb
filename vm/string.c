#include <stdarg.h>
#include <stdio.h>
#include "tr.h"
#include "internal.h"

static OBJ TrSymbol_lookup(VM, const char *str) {
  khash_t(str) *kh = vm->symbols;
  khiter_t k = kh_get(str, kh, str);
  if (k != kh_end(kh)) return kh_value(kh, k);
  return TR_NIL;
}

static void TrSymbol_add(VM, const char *str, OBJ id) {
  int ret;
  khash_t(str) *kh = vm->symbols;
  khiter_t k = kh_put(str, kh, str, &ret);
  if (!ret) kh_del(str, kh, k);
  kh_value(kh, k) = id;
}

OBJ TrSymbol_new(VM, const char *str) {
  OBJ id = TrSymbol_lookup(vm, str);
  
  if (!id) {
    TrSymbol *s = TR_INIT_OBJ(Symbol);
    s->len = strlen(str);
    s->ptr = TR_ALLOC_N(char, s->len+1);
    s->interned = 1;
    TR_MEMCPY_N(s->ptr, str, char, s->len);
    s->ptr[s->len] = '\0';
    
    id = (OBJ)s;
    TrSymbol_add(vm, s->ptr, id);
  }
  return id;
}

static OBJ TrSymbol_to_s(VM, OBJ self) {
  return TrString_new(vm, TR_STR_PTR(self), TR_STR_LEN(self));
}

static OBJ TrString_to_s(VM, OBJ self) {
  return self;
}

static OBJ TrString_size(VM, OBJ self) {
  return TrFixnum_new(vm, TR_CSTRING(self)->len);
}

OBJ TrString_new(VM, const char *str, size_t len) {
  TrString *s = TR_INIT_OBJ(String);
  s->len = len;
  s->ptr = TR_ALLOC_N(char, s->len+1);
  s->interned = 0;
  TR_MEMCPY_N(s->ptr, str, char, s->len);
  s->ptr[s->len] = '\0';
  return (OBJ)s;
}

OBJ TrString_new2(VM, const char *str) {
  return TrString_new(vm, str, strlen(str));
}

OBJ TrString_new3(VM, size_t len) {
  TrString *s = TR_INIT_OBJ(String);
  s->len = len;
  s->ptr = TR_ALLOC_N(char, s->len+1);
  s->interned = 0;
  s->ptr[s->len] = '\0';
  return (OBJ)s;
}

OBJ TrString_concat(VM, OBJ self, OBJ other) {
  return tr_sprintf(vm, "%s%s", TR_STR_PTR(self), TR_STR_PTR(other));
}

OBJ tr_sprintf(VM, const char *fmt, ...) {
  va_list arg;
  va_start(arg, fmt);
  int len = vsnprintf(NULL, 0, fmt, arg);
  char *ptr = TR_ALLOC_N(char, len);
  va_end(arg);
  va_start(arg, fmt);
  vsprintf(ptr, fmt, arg);
  va_end(arg);
  /* TODO do not allocate twice */
  OBJ str = TrString_new(vm, ptr, len);
  TR_FREE(ptr);
  return str;
}

void TrSymbol_init(VM) {
  OBJ c = TR_INIT_CLASS(Symbol, Object);
  tr_def(c, "to_s", TrSymbol_to_s, 0);
}

void TrString_init(VM) {
  OBJ c = TR_INIT_CLASS(String, Object);
  tr_def(c, "to_s", TrString_to_s, 0);
  tr_def(c, "size", TrString_size, 0);
  tr_def(c, "length", TrString_size, 0);
  tr_def(c, "+", TrString_concat, 1);
}
