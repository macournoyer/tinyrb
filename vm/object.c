#include "tinyrb.h"

void tr_obj_init(VM, tr_type type, OBJ obj, OBJ class)
{
  tr_obj *o = TR_COBJ(obj);
  
  o->type      = type;
  o->ivars     = tr_hash_new(vm);
  o->class     = TR_CCLASS(class);
  o->metaclass = (tr_class *) tr_metaclass_new(vm);
}

OBJ tr_new(VM, OBJ class)
{
  tr_obj *obj = (tr_obj *) tr_malloc(sizeof(tr_obj));
  tr_obj_init(vm, TR_OBJECT, (OBJ) obj, class);
  
  return (OBJ) obj;
}

static OBJ tr_object_inspect(VM, OBJ self)
{
  OBJ str = tr_string_new(vm, "#<Object:0x000000>");
  
  /* TODO buffer overflow!!! */
  sprintf(TR_STR(str), "#<%s:0x%d>", TR_STR(TR_COBJ(self)->class->name), (int) self);
  return str;
}

static OBJ tr_object_class(VM, OBJ self)
{
  return (OBJ) TR_COBJ(self)->class;
}

void tr_object_init(VM)
{
  OBJ object = tr_class_new(vm, "Object", TR_NIL);
  
  tr_def(vm, object, "inspect", tr_object_inspect, 0);
  tr_def(vm, object, "to_s", tr_object_inspect, 0);
  tr_def(vm, object, "class", tr_object_class, 0);
}
