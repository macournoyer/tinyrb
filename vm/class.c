#include "tinyrb.h"

void tr_const_set(VM, const char *name, OBJ obj)
{
  tr_hash_set(vm, CUR_FRAME->consts, tr_intern(vm, name), obj);
}

OBJ tr_const_get(VM, const char *name)
{
  return tr_hash_get(vm, CUR_FRAME->consts, tr_intern(vm, name));
}

static OBJ tr_lookup_method(VM, OBJ obj, OBJ name)
{
  OBJ met;
  
  /* lookup in metaclass methods */
  met = tr_hash_get(vm, TR_COBJ(obj)->metaclass->methods, name);
  if (met != TR_NIL)
    return met;
  
  /* lookup in class methods */
  met = tr_hash_get(vm, TR_COBJ(obj)->class->methods, name);
  if (met != TR_NIL)
    return met;
  
  /* TODO decend class->super */
  
  /* method not found */
  /* TODO call method_missing */
  return TR_NIL;
}

OBJ tr_send(VM, OBJ obj, OBJ message, int argc, OBJ argv[])
{
  OBJ met = tr_lookup_method(vm, obj, message);
  
  if (met != TR_NIL) {
    tr_method *m = TR_CMETHOD(met);
    
    if (m->argc != argc) {
      tr_log("wrong number of arguments: %d for %d", argc, m->argc);
      return TR_UNDEF;
    }
        
    /* HACK better way to have variable num of args? */
    return m->func(vm, obj, argv[0], argv[1], argv[2], argv[3], argv[4],
                            argv[5], argv[6], argv[7], argv[8], argv[9]);
  } else {
    tr_log("method not found: %s", TR_STR(message));
    return TR_UNDEF;
  }
}

static OBJ tr_class_def(VM, tr_class *class, const char *name, OBJ (*func)(), int argc)
{
  tr_method *met = (tr_method *) tr_malloc(sizeof(tr_method));
  
  met->type = TR_METHOD;
  met->name = tr_intern(vm, name);
  met->func = func;
  met->argc = argc;
  
  tr_hash_set(vm, class->methods, met->name, (OBJ) met);
  
  return TR_NIL;
}

OBJ tr_def(VM, OBJ obj, const char *name, OBJ (*func)(), int argc)
{
  return tr_class_def(vm, TR_CCLASS(obj), name, func, argc);
}

OBJ tr_metadef(VM, OBJ obj, const char *name, OBJ (*func)(), int argc)
{
  return tr_class_def(vm, TR_COBJ(obj)->metaclass, name, func, argc);
}

static tr_class *tr_metaclass_new(VM)
{
  tr_class *c = (tr_class *) tr_malloc(sizeof(tr_class));
  
  c->type      = TR_CLASS;
  c->name      = tr_intern(vm, "");
  c->super     = (tr_class *) TR_NIL;
  c->ivars     = tr_hash_new(vm);
  c->methods   = tr_hash_new(vm);
  c->class     = (tr_class *) TR_NIL;
  c->metaclass = (tr_class *) TR_NIL;
  
  return c;
}

OBJ tr_class_new(VM, const char* name, OBJ super)
{
  tr_class *c = (tr_class *) tr_malloc(sizeof(tr_class));
  
  c->type      = TR_CLASS;
  c->name      = tr_intern(vm, name);
  c->super     = (tr_class *) super;
  c->ivars     = tr_hash_new(vm);
  c->methods   = tr_hash_new(vm);
  c->class     = (tr_class *) tr_const_get(vm, "Class");
  c->metaclass = tr_metaclass_new(vm);
  
  tr_const_set(vm, name, (OBJ) c);
  
  return (OBJ) c;
}

void tr_obj_init(VM, tr_type type, OBJ obj, OBJ class)
{
  tr_obj *o = TR_COBJ(obj);
  
  o->type      = type;
  o->ivars     = tr_hash_new(vm);
  o->class     = TR_CCLASS(class);
  o->metaclass = tr_metaclass_new(vm);
}

OBJ tr_new(VM, OBJ class)
{
  tr_obj *obj = (tr_obj *) tr_malloc(sizeof(tr_obj));
  tr_obj_init(vm, TR_OBJECT, (OBJ) obj, class);
  
  return (OBJ) obj;
}