#include "tr.h"
#include "internal.h"

OBJ TrObject_new(VM) {
  return (OBJ) TR_INIT_OBJ(Object);
}

OBJ TrObject_method(VM, OBJ self, OBJ name) {
  TrObject *o = TR_COBJECT(self);
  /* TODO lookup in metaclass */
  return TrClass_lookup(vm, o->class, name);
}

/* TODO respect namespace */
OBJ TrObject_const_get(VM, OBJ self, OBJ name) {
  khiter_t k = kh_get(OBJ, vm->consts, name);
  if (k != kh_end(vm->consts)) return kh_value(vm->consts, k);
  return TR_NIL;
}

OBJ TrObject_const_set(VM, OBJ self, OBJ name, OBJ value) {
  int ret;
  khiter_t k = kh_put(OBJ, vm->consts, name, &ret);
  if (!ret) kh_del(OBJ, vm->consts, k);
  kh_value(vm->consts, k) = value;
  return value;
}

static OBJ TrObject_class(VM, OBJ self) {
  return TR_COBJECT(self)->class;
}

static OBJ TrObject_inspect(VM, OBJ self) {
  return tr_sprintf(vm, "#<Object:%p>", (void*)self);
}

static OBJ TrObject_puts(VM, OBJ self, int argc, OBJ argv[]) {
  size_t i;
  for (i = 0; i < argc; ++i)
    printf("%s\n", TR_STR_PTR(tr_send2(argv[i], "to_s")));
  return TR_NIL;
}

void TrObject_init(VM) {
  OBJ c = TR_INIT_CLASS(Object, /* ignored */ Object);
  tr_def(c, "class", TrObject_class, 0);
  tr_def(c, "puts", TrObject_puts, -1);
  tr_def(c, "to_s", TrObject_inspect, 0);
  tr_def(c, "inspect", TrObject_inspect, 0);
}
