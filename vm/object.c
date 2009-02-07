#include "tr.h"

OBJ TrObject_new(VM) {
  return (OBJ) TR_INIT_OBJ(Object);
}

OBJ TrObject_method(VM, OBJ self, OBJ name) {
  TrObject *o = TR_COBJECT(self);
  /* TODO lookup in metaclass */
  return TrClass_lookup(vm, o->class, name);
}

static OBJ TrObject_class(VM, OBJ self) {
  return TR_COBJECT(self)->class;
}

static OBJ TrObject_puts(VM, OBJ self, OBJ i) {
  printf("%s\n", TR_STR_PTR(tr_send2(i, "to_s")));
  return TR_NIL;
}

void TrObject_init(VM) {
  OBJ c = TR_INIT_CLASS(Object, /* ignored */ Object);
  tr_def(c, "class", TrObject_class);
  tr_def(c, "puts", TrObject_puts);
}
