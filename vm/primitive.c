#include "tr.h"
#include "internal.h"

OBJ TrNil_to_s(VM, OBJ self) {
  return TrString_new2(vm, "");
}

OBJ TrTrue_to_s(VM, OBJ self) {
  return TrString_new2(vm, "true");
}

OBJ TrFalse_to_s(VM, OBJ self) {
  return TrString_new2(vm, "false");
}

void TrPrimitive_init(VM) {
  OBJ nilc = TR_INIT_CLASS(NilClass, Object);
  OBJ truec = TR_INIT_CLASS(TrueClass, Object);
  OBJ falsec = TR_INIT_CLASS(FalseClass, Object);
  
  vm->primitives[TR_NIL] = TrClass_allocate(vm, nilc);
  vm->primitives[TR_TRUE] = TrClass_allocate(vm, truec);
  vm->primitives[TR_FALSE] = TrClass_allocate(vm, falsec);
  
  tr_def(nilc, "to_s", TrNil_to_s, 0);
  tr_def(truec, "to_s", TrTrue_to_s, 0);
  tr_def(falsec, "to_s", TrFalse_to_s, 0);
}