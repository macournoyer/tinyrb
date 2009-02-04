#include <stdio.h>
#include "tr.h"

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
    TR_MEMCPY_N(s->ptr, str, char, s->len);
    s->ptr[s->len] = '\0';
    
    id = (OBJ)s;
    TrSymbol_add(vm, s->ptr, id);
  }
  return id;
}

static OBJ TrSymbol_display(VM, OBJ self) {
  printf("%s\n", TR_STR_PTR(self));
  return self;
}

void TrSymbol_init(VM) {
  OBJ c = vm->classes[TR_T_Symbol] = TrClass_new(vm, tr_intern("Symbol"), TR_NIL);
  tr_def(c, "display", TrSymbol_display);
}
