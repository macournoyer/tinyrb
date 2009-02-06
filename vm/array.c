#include <stdarg.h>
#include "tr.h"

OBJ TrArray_new(VM) {
  TrArray *a = TR_INIT_OBJ(Array);
  kv_init(a->kv);
  return (OBJ)a;
}

OBJ TrArray_new2(VM, int argc, ...) {
  OBJ a = TrArray_new(vm);
  va_list argp;
  size_t  i;
  va_start(argp, argc);
  for (i = 0; i < argc; ++i)
    TR_ARRAY_PUSH(a, va_arg(argp, OBJ));
  va_end(argp);
  return a;
}

void TrArray_init(VM) {
  TR_INIT_CLASS(Array, Object);
}