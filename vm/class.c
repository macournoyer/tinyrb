#include "tr.h"

/* class */

OBJ TrClass_new(VM, OBJ name, OBJ super) {
  TrClass *c = TR_INIT_OBJ(Class);
  c->name = name;
  c->super = super;
  c->methods = kh_init(OBJ);
  return (OBJ)c;
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

/* method */

OBJ TrMethod_new(VM, TrFunc *func, OBJ data) {
  TrMethod *m = TR_INIT_OBJ(Method);
  m->func = func;
  m->data = data;
  return (OBJ)m;
}
