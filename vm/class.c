#include "tinyrb.h"

/* class */

static OBJ tr_class_def(VM, tr_class *class, const char *name, OBJ (*func)(), int argc)
{
  tr_method *met = (tr_method *) tr_malloc(sizeof(tr_method));
  
  met->type = TR_METHOD;
  met->name = tr_intern(vm, name);
  met->func = func;
  met->ops  = TR_NIL;
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

OBJ tr_ops_def(VM, OBJ class, OBJ name, OBJ ops)
{
  tr_method *met = (tr_method *) tr_malloc(sizeof(tr_method));
  
  met->type = TR_METHOD;
  met->name = name;
  met->func = NULL;
  met->ops  = ops;
  met->argc = -1;
  
  tr_hash_set(vm, TR_CCLASS(class)->methods, met->name, (OBJ) met);
  
  return TR_NIL;
}

OBJ tr_metaclass_new(VM)
{
  tr_class *c = (tr_class *) tr_malloc(sizeof(tr_class));
  
  c->type      = TR_CLASS;
  c->name      = tr_intern(vm, "");
  c->super     = (tr_class *) TR_NIL;
  c->ivars     = tr_hash_new(vm);
  c->methods   = tr_hash_new(vm);
  c->class     = (tr_class *) TR_NIL;
  c->metaclass = (tr_class *) TR_NIL;
  
  return (OBJ) c;
}

OBJ tr_class_new(VM, const char* name, OBJ super)
{
  tr_class *c = (tr_class *) tr_malloc(sizeof(tr_class));
  
  c->type      = TR_CLASS;
  c->name      = tr_intern(vm, name);
  c->super     = (tr_class *) super;
  c->ivars     = tr_hash_new(vm);
  c->methods   = tr_hash_new(vm);
  c->metaclass = (tr_class *) tr_metaclass_new(vm);
  c->modules   = tr_array_struct(vm);
  if (strcmp(name, "Class") != 0) /* HACK */
    c->class   = (tr_class *) tr_const_get(vm, "Class");
  
  tr_const_set(vm, name, (OBJ) c);
  
  return (OBJ) c;
}

OBJ tr_class_define(VM, OBJ name, OBJ cbase, OBJ super, OBJ ops, int define_type)
{
  OBJ   class;
  char *n = TR_STR(name);
  
  if (tr_const_defined(vm, n))
    class = tr_const_get(vm, n);
  else
    class = tr_class_new(vm, n, super == TR_NIL ? tr_const_get(vm, "Object") : super);
  
  tr_next_frame(vm, class, class);
  OBJ ret = tr_run(vm, ops);
  tr_prev_frame(vm);
  
  return ret;
}

static OBJ tr_class_name(VM, OBJ self)
{
  return (OBJ) TR_CCLASS(self)->name;
}

void tr_class_init(VM)
{
  OBJ class = tr_class_new(vm, "Class", TR_NIL);
  
  tr_def(vm, class, "name", tr_class_name, 0);
  tr_def(vm, class, "new", tr_new, -1);
}

/* module */

OBJ tr_module_new(VM, const char* name)
{
  tr_class *c = (tr_class *) tr_malloc(sizeof(tr_class));
  
  c->type      = TR_CLASS;
  c->name      = tr_intern(vm, name);
  c->super     = NULL;
  c->ivars     = tr_hash_new(vm);
  c->methods   = tr_hash_new(vm);
  c->metaclass = (tr_class *) tr_metaclass_new(vm);
  c->modules   = tr_array_struct(vm);
  c->class     = (tr_class *) tr_const_get(vm, "Module");
  
  tr_const_set(vm, name, (OBJ) c);
  
  return (OBJ) c;
}

OBJ tr_module_include(VM, OBJ self, OBJ module)
{
  tr_obj *obj = TR_COBJ(self);
  
  tr_array_push(vm, (OBJ) obj->modules, module);
  
  return TR_NIL;
}

void tr_module_init(VM)
{
  OBJ module = tr_class_new(vm, "Module", tr_const_get(vm, "Object"));
  
  tr_metadef(vm, module, "include", tr_module_include, 1);
}