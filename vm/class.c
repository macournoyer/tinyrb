#include "tr.h"
#include "internal.h"

#define TR_INIT_MODULE(M) \
  (M)->name = name; \
  (M)->methods = kh_init(OBJ); \
  kv_init((M)->modules)

/* module */

OBJ TrModule_new(VM, OBJ name) {
  TrModule *m = TR_INIT_OBJ(Module);
  TR_INIT_MODULE(m);
  return (OBJ)m;
}

OBJ TrModule_lookup(VM, OBJ self, OBJ name) {
  TrModule *m = TR_CMODULE(self);
  /* lookup in self */
  khiter_t k = kh_get(OBJ, m->methods, name);
  if (k != kh_end(m->methods)) return kh_value(m->methods, k);
  /* lookup in included modules */
  size_t i;
  for (i = 0; i < kv_size(m->modules); ++i) {
    OBJ ret = TrModule_lookup(vm, kv_A(m->modules, i), name);
    if (ret) return ret;
  }
  return TR_NIL;
}

OBJ TrModule_add_method(VM, OBJ self, OBJ name, OBJ method) {
  TrClass *m = TR_CMODULE(self);
  int ret;
  khiter_t k = kh_put(OBJ, m->methods, name, &ret);
  if (!ret) kh_del(OBJ, m->methods, k);
  kh_value(m->methods, k) = method;
  TR_CMETHOD(method)->name = name;
  return method;
}

OBJ TrModule_include(VM, OBJ self, OBJ mod) {
  TrClass *m = TR_CMODULE(self);
  kv_push(OBJ, m->modules, mod);
  return mod;
}

static OBJ TrModule_name(VM, OBJ self) {
  return TR_CMODULE(self)->name;
}

void TrModule_init(VM) {
  OBJ c = TR_INIT_CLASS(Module, Object);
  tr_def(c, "name", TrModule_name, 0);
  tr_def(c, "include", TrModule_include, 1);
  tr_def(c, "to_s", TrModule_name, 0);
}

/* class */

OBJ TrClass_new(VM, OBJ name, OBJ super) {
  TrClass *c = TR_INIT_OBJ(Class);
  TR_INIT_MODULE(c);
  c->super = super;
  return (OBJ)c;
}

OBJ TrClass_allocate(VM, OBJ self) {
  TrObject *o = TR_INIT_OBJ(Object);
  o->class = self;
  return (OBJ)o;
}

OBJ TrClass_lookup(VM, OBJ self, OBJ name) {
  OBJ m = TrModule_lookup(vm, self, name);
  if (m) return m;
  TrClass *c = TR_CCLASS(self);
  if (c->super) return TrClass_lookup(vm, c->super, name);
  return TR_NIL;
}

OBJ TrClass_superclass(VM, OBJ self) {
  return TR_CCLASS(self)->super;
}

void TrClass_init(VM) {
  OBJ c = TR_INIT_CLASS(Class, Module);
  tr_def(c, "superclass", TrClass_superclass, 0);
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

OBJ TrMethod_name(VM, OBJ self) { return TR_CMETHOD(self)->name; }
OBJ TrMethod_arity(VM, OBJ self) { return TrFixnum_new(vm, TR_CMETHOD(self)->arity); }

void TrMethod_init(VM) {
  OBJ c = TR_INIT_CLASS(Method, Object);
  tr_def(c, "name", TrMethod_name, 0);
  tr_def(c, "arity", TrMethod_arity, 0);
}
