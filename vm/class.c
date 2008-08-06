#include "tinyrb.h"

void tr_const_set(VM, const char *name, OBJ obj)
{
  tr_hash_set(CUR_FRAME->consts, tr_intern(name), obj);
}

OBJ tr_const_get(VM, const char *name)
{
  return tr_hash_get(CUR_FRAME->consts, tr_intern(name));
}

static OBJ tr_lookup_method(OBJ obj, OBJ name)
{
  OBJ met;
  
  /* lookup in metaclass methods */
  met = tr_hash_get(TR_COBJ(obj)->metaclass->methods, name);
  if (met != TR_NIL)
    return met;
  
  /* lookup in class methods */
  met = tr_hash_get(TR_COBJ(obj)->class->methods, name);
  if (met != TR_NIL)
    return met;
  
  /* TODO decend class->super */
  
  /* method not found */
  /* TODO call method_missing */
  return TR_NIL;
}

OBJ tr_send(OBJ obj, OBJ message, int argc, OBJ argv[])
{
  OBJ met = tr_lookup_method(obj, message);
  
  if (met != TR_NIL) {
    /* TODO handle multiple args & check argc */
    return TR_CMETHOD(met)->func(argv[0]);
  } else {
    tr_log("method not found: %s", TR_STR(message));
    return TR_UNDEF;
  }
}

static OBJ tr_class_def(tr_class *class, const char *name, OBJ (*func)(), int argc)
{
  tr_method *met = (tr_method *) tr_malloc(sizeof(tr_method));
  
  met->type = TR_METHOD;
  met->name = tr_intern(name);
  met->func = func;
  met->argc = argc;
  
  tr_hash_set(class->methods, met->name, (OBJ) met);
  
  return TR_NIL;
}

OBJ tr_def(OBJ obj, const char *name, OBJ (*func)(), int argc)
{
  return tr_class_def(TR_CCLASS(obj), name, func, argc);
}

OBJ tr_metadef(OBJ obj, const char *name, OBJ (*func)(), int argc)
{
  return tr_class_def(TR_COBJ(obj)->metaclass, name, func, argc);
}

static tr_class *tr_metaclass_new()
{
  tr_class *c = (tr_class *) tr_malloc(sizeof(tr_class));
  
  c->type      = TR_CLASS;
  c->name      = tr_intern("");
  c->super     = (tr_class *) TR_NIL;
  c->ivars     = tr_hash_new();
  c->methods   = tr_hash_new();
  c->class     = (tr_class *) TR_NIL;
  c->metaclass = (tr_class *) TR_NIL;
  
  return c;
}

OBJ tr_class_new(VM, const char* name, OBJ super)
{
  tr_class *c = (tr_class *) tr_malloc(sizeof(tr_class));
  
  c->type      = TR_CLASS;
  c->name      = tr_intern(name);
  c->super     = (tr_class *) super;
  c->ivars     = tr_hash_new();
  c->methods   = tr_hash_new();
  c->class     = (tr_class *) tr_const_get(vm, "Class");
  c->metaclass = tr_metaclass_new();
  
  tr_const_set(vm, name, (OBJ) c);
  
  return (OBJ) c;
}

OBJ tr_new(OBJ class)
{
  tr_obj *obj = (tr_obj *) tr_malloc(sizeof(tr_obj));
  
  obj->type      = TR_OBJECT;
  obj->ivars     = tr_hash_new();
  obj->class     = TR_CCLASS(class);
  obj->metaclass = tr_metaclass_new();
  
  return (OBJ) obj;
}