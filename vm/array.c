#include <stdarg.h>
#include "tr.h"
#include "internal.h"

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

static OBJ TrArray_length(VM, OBJ self) {
  return TrFixnum_new(vm, TR_ARRAY_SIZE(self));
}

void TrArray_init(VM) {
  OBJ c = TR_INIT_CLASS(Array, Object);
  tr_def(c, "length", TrArray_length, 0);
}
