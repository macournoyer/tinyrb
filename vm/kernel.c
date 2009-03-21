#include "tr.h"
#include "internal.h"

OBJ TrBinding_new(VM, TrFrame *f) {
  TrBinding *b = TR_INIT_OBJ(Binding);
  b->frame = f;
  return (OBJ)b;
}

void TrBinding_init(VM) {
  TR_INIT_CLASS(Binding, Object);
}

static OBJ TrKernel_puts(VM, OBJ self, int argc, OBJ argv[]) {
  size_t i;
  for (i = 0; i < argc; ++i) printf("%s\n", TR_STR_PTR(tr_send2(argv[i], "to_s")));
  return TR_NIL;
}

static OBJ TrKernel_binding(VM, OBJ self) {
  return TrBinding_new(vm, PREV_FRAME);
}

static OBJ TrKernel_eval(VM, OBJ self, int argc, OBJ argv[]) {
  if (argc < 1) tr_raise("string argument required");
  if (argc > 4) tr_raise("Too much arguments");
  OBJ string = argv[0];
  TrFrame *f = (argc > 1 && argv[1]) ? TR_CBINDING(argv[1])->frame : FRAME;
  char *filename = (argc > 2 && argv[1]) ? TR_STR_PTR(argv[2]) : "<eval>";
  size_t lineno = argc > 3 ? TR_FIX2INT(argv[3]) : 0;
  TrBlock *blk = TrBlock_compile(vm, TR_STR_PTR(string), filename, lineno);
  if (vm->debug) TrBlock_dump(vm, blk);
  return TrVM_run(vm, blk, f->self, f->class, kv_size(blk->locals), f->stack);
}

static OBJ TrKernel_load(VM, OBJ self, OBJ filename) {
  return TrVM_load(vm, TR_STR_PTR(filename));
}

static OBJ TrKernel_raise(VM, OBJ self, OBJ exception) {
  TrVM_raise(vm, exception);
  return TR_NIL;
}

void TrKernel_init(VM) {
  OBJ m = tr_defmodule("Kernel");
  TrModule_include(vm, TR_CLASS(Object), m);
  tr_def(m, "puts", TrKernel_puts, -1);
  tr_def(m, "eval", TrKernel_eval, -1);
  tr_def(m, "load", TrKernel_load, 1);
  tr_def(m, "binding", TrKernel_binding, 0);
  tr_def(m, "raise", TrKernel_raise, 1);
}
