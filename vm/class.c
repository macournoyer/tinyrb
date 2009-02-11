#include "tr.h"
#include "internal.h"

/* class */

OBJ TrClass_new(VM, OBJ name, OBJ super) {
  TrClass *c = TR_INIT_OBJ(Class);
  c->name = name;
  c->super = super;
  c->methods = kh_init(OBJ);
  return (OBJ)c;
}

OBJ TrClass_allocate(VM, OBJ self) {
  TrObject *o = TR_INIT_OBJ(Object);
  o->class = self;
  return (OBJ)o;
}

OBJ TrClass_lookup(VM, OBJ self, OBJ name) {
  TrClass *c = TR_CCLASS(self);
  khiter_t k = kh_get(OBJ, c->methods, name);
  if (k != kh_end(c->methods)) return kh_value(c->methods, k);
  if (c->super) return TrClass_lookup(vm, c->super, name);
  return TR_NIL;
}

OBJ TrClass_add_method(VM, OBJ self, OBJ name, OBJ method) {
  TrClass *c = TR_CCLASS(self);
  int ret;
  khiter_t k = kh_put(OBJ, c->methods, name, &ret);
  if (!ret) kh_del(OBJ, c->methods, k);
  kh_value(c->methods, k) = method;
  return method;
}

OBJ TrClass_name(VM, OBJ self) {
  return TR_CCLASS(self)->name;
}

void TrClass_init(VM) {
  OBJ c = TR_INIT_CLASS(Class, Object);
  tr_def(c, "name", TrClass_name, 0);
  tr_def(c, "allocate", TrClass_allocate, 0);
}

/* method */

OBJ TrMethod_new(VM, TrFunc *func, OBJ data, int arity) {
  TrMethod *m = TR_INIT_OBJ(Method);
  m->func = func;
  m->data = data;
  m->arity = arity;
  return (OBJ)m;
}
