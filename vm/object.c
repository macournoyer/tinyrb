#include "tr.h"

OBJ TrObject_method(VM, OBJ self, OBJ name) {
  TrObject *o = TR_COBJECT(self);
  /* TODO lookup in metaclass */
  return TrClass_lookup(vm, o->class, name);
}

OBJ TrObject_class(VM, OBJ self) {
  return TR_COBJECT(self)->class;
}

void TrObject_init(VM) {
  OBJ c = TR_INIT_CLASS(Object, /* ignored */ Object);
  tr_def(c, "class", TrObject_class);
}
