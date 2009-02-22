#include "tr.h"
#include "internal.h"

static OBJ TrKernel_puts(VM, OBJ self, int argc, OBJ argv[]) {
  size_t i;
  for (i = 0; i < argc; ++i) printf("%s\n", TR_STR_PTR(tr_send2(argv[i], "to_s")));
  return TR_NIL;
}

void TrKernel_init(VM) {
  OBJ m = tr_defmodule("Kernel");
  TrModule_include(vm, TR_CLASS(Object), m);
  tr_def(m, "puts", TrKernel_puts, -1);
}

