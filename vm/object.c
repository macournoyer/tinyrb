#include "tinyrb.h"

/* constants */

void tr_const_set(VM, const char *name, OBJ obj)
{
  tr_hash_set(vm, CUR_FRAME->consts, tr_intern(vm, name), obj);
}

OBJ tr_const_get(VM, const char *name)
{
  OBJ c = tr_hash_get(vm, CUR_FRAME->consts, tr_intern(vm, name));
  
  if (c == TR_NIL)
    tr_raise(vm, "Constant not found: %s", name);
  
  return c;
}

int tr_const_defined(VM, const char *name)
{
  OBJ c = tr_hash_get(vm, CUR_FRAME->consts, tr_intern(vm, name));
  
  return c != TR_NIL;
}

OBJ tr_special_get(VM, OBJ obj)
{
  switch (obj) {
    case TR_NIL: return tr_const_get(vm, "NIL");
    case TR_TRUE: return tr_const_get(vm, "TRUE");
    case TR_FALSE: return tr_const_get(vm, "FALSE");
  }
  return obj;
}

/* method */

static OBJ tr_lookup_method_in_modules(VM, OBJ obj, OBJ name, OBJ class)
{
  tr_class       *c = TR_CCLASS(class);
  tr_array_entry *e = c->modules->first;
  OBJ             met;
  
  while (e != NULL) {
    met = tr_hash_get(vm, TR_CCLASS(e->value)->methods, name);
    if (met != TR_NIL)
      return met;
    e = e->next;
  }
  
  return TR_NIL;
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
  
  met = tr_lookup_method_in_modules(vm, obj, name, (OBJ) TR_COBJ(obj)->class);
  if (met != TR_NIL)
    return met;
  
  /* decend class->super */
  tr_class *class = TR_COBJ(obj)->class->super;
  while (TR_NIL != (OBJ) class) {
    met = tr_hash_get(vm, class->methods, name);
    if (met != TR_NIL)
      return met;
    
    met = tr_lookup_method_in_modules(vm, obj, name, (OBJ) class);
    if (met != TR_NIL)
      return met;
    
    class = class->super;
  }
  
  /* method not found */
  /* TODO call method_missing */
  tr_raise(vm, "method not found: %s on %s", TR_STR(name), TR_STR(tr_object_inspect(vm, obj)));
  
  return TR_NIL;
}

OBJ tr_send(VM, OBJ obj, OBJ message, int argc, OBJ argv[], OBJ block_ops)
{
             obj = tr_special_get(vm, obj);
  OBJ        met = tr_lookup_method(vm, obj, message);
  tr_method *m   = TR_CMETHOD(met);
  
  CUR_FRAME->block = TR_NIL;
  
  if (m->func) { /* C based method */
    if (block_ops != TR_NIL)
      CUR_FRAME->block = tr_proc_new(vm, block_ops);
    
    if (m->argc == -1) { /* varargs */
      return m->func(vm, obj, argc, argv);
    } else {
      if (m->argc != argc)
        tr_raise(vm, "wrong number of arguments: %d for %d", argc, m->argc);
      
      /* HACK better way to have variable num of args? */
      return m->func(vm, obj, argv[0], argv[1], argv[2], argv[3], argv[4],
                              argv[5], argv[6], argv[7], argv[8], argv[9]);
    }
    
  } else { /* opcode based method */
    size_t i;
    OBJ    ret;
    
    tr_next_frame(vm, obj, (OBJ) TR_COBJ(obj)->class);
    
    if (block_ops != TR_NIL)
      CUR_FRAME->block = tr_proc_new(vm, block_ops);
    
    /* move method args to locals */
    for (i = 0; i < argc; ++i)
      tr_hash_set(vm, CUR_FRAME->locals, tr_fixnum_new(vm, argc-i+1), argv[i]);
    
    ret = tr_run(vm, m->ops);
    
    tr_prev_frame(vm);
    return ret;
    
  }
}

/* object */

void tr_obj_init(VM, tr_type type, OBJ obj, OBJ class)
{
  tr_obj *o = TR_COBJ(obj);
  
  o->type      = type;
  o->ivars     = tr_hash_new(vm);
  o->class     = TR_CCLASS(class);
  o->metaclass = (tr_class *) tr_metaclass_new(vm);
  o->modules   = tr_array_struct(vm);
}

OBJ tr_new(VM, OBJ class, int argc, OBJ argv[])
{
  OBJ obj = (OBJ) tr_malloc(sizeof(tr_obj));

  tr_obj_init(vm, TR_OBJECT, obj, class);
  
  tr_send(vm, obj, tr_intern(vm, "initialize"), argc, argv, TR_NIL);
  
  return obj;
}

OBJ tr_new2(VM, OBJ class)
{
  OBJ argv[0];
  return tr_new(vm, class, 0, argv);
}

OBJ tr_object_inspect(VM, OBJ self)
{
  OBJ str = tr_string_new(vm, "#<Object:0x000000>");
  
  /* TODO buffer overflow!!! */
  sprintf(TR_STR(str), "#<%s:%p>", TR_STR(TR_COBJ(self)->class->name), (int) self);
  return str;
}

static OBJ tr_object_class(VM, OBJ self)
{
  return (OBJ) TR_COBJ(self)->class;
}

static OBJ tr_object_nop(VM, OBJ self)
{
  return self;
}

void tr_object_init(VM)
{
  OBJ object = tr_class_new(vm, "Object", TR_NIL);
  
  tr_def(vm, object, "inspect", tr_object_inspect, 0);
  tr_def(vm, object, "to_s", tr_object_inspect, 0);
  tr_def(vm, object, "class", tr_object_class, 0);
  tr_def(vm, object, "initialize", tr_object_nop, -1);
}

/* special objects (true, false, nil) */

static OBJ tr_nil_to_s(VM, OBJ self)
{
  return tr_string_new(vm, "");
}

static OBJ tr_true_to_s(VM, OBJ self)
{
  return tr_string_new(vm, "true");
}

static OBJ tr_false_to_s(VM, OBJ self)
{
  return tr_string_new(vm, "false");
}

void tr_special_init(VM)
{
  OBJ objectclass = tr_const_get(vm, "Object");
  
  OBJ nilclass = tr_class_new(vm, "NilClass", objectclass);
  tr_def(vm, nilclass, "to_s", tr_nil_to_s, 0);
  tr_const_set(vm, "NIL", tr_new2(vm, nilclass));
  
  OBJ trueclass = tr_class_new(vm, "TrueClass", objectclass);
  tr_def(vm, trueclass, "to_s", tr_true_to_s, 0);
  tr_const_set(vm, "TRUE", tr_new2(vm, trueclass));
  
  OBJ falseclass = tr_class_new(vm, "FalseClass", objectclass);
  tr_def(vm, falseclass, "to_s", tr_false_to_s, 0);
  tr_const_set(vm, "FALSE", tr_new2(vm, falseclass));
}