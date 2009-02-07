#include "tr.h"

OBJ TrFixnum_new(VM, int value) {
  TrFixnum *n = TR_INIT_OBJ(Fixnum);
  n->value = value;
  return (OBJ)n;
}

OBJ TrFixnum_add(VM, OBJ self, OBJ other) {
  return INT2FIX(TR_CFIXNUM(self)->value + FIX2INT(other));
}

OBJ TrFixnum_to_s(VM, OBJ self) {
  return tr_sprintf(vm, "%d", TR_CFIXNUM(self)->value);
}

void TrFixnum_init(VM) {
  OBJ c = TR_INIT_CLASS(Fixnum, Object);
  tr_def(c, "+", TrFixnum_add);
  tr_def(c, "to_s", TrFixnum_to_s);
}
