#include "tr.h"

OBJ TrObject_method(VM, OBJ self, OBJ name) {
  TrObject *o = TR_COBJECT(self);
  /* TODO lookup in metaclass */
  return TrClass_lookup(vm, o->class, name);
}

void TrObject_init(VM) {
  
}
