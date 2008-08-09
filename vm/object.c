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

OBJ tr_special_get(VM, OBJ obj)
{
  switch (obj) {
    case TR_NIL: return tr_const_get(vm, "NIL");
    case TR_TRUE: return tr_const_get(vm, "TRUE");
    case TR_FALSE: return tr_const_get(vm, "FALSE");
  }
  return obj;
}

/* object */

void tr_obj_init(VM, tr_type type, OBJ obj, OBJ class)
{
  tr_obj *o = TR_COBJ(obj);
  
  o->type      = type;
  o->ivars     = tr_hash_new(vm);
  o->class     = TR_CCLASS(class);
  o->metaclass = (tr_class *) tr_metaclass_new(vm);
}

OBJ tr_new(VM, OBJ class, int argc, OBJ argv[])
{
  OBJ obj = (OBJ) tr_malloc(sizeof(tr_obj));

  tr_obj_init(vm, TR_OBJECT, obj, class);
  
  tr_send(vm, obj, tr_intern(vm, "initialize"), argc, argv);
  
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