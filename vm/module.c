#include "tinyrb.h"

OBJ tr_call(VM, OBJ obj, const char *method)
{
  tr_module *mod = (tr_module *) obj;
  /* TODO look in imethods */
  tr_method *met = (tr_method *) tr_hash_get(mod->methods, (void *) method);
  
  /* TODO handle multiple args */
  if (met) {
    return met->func();
  } else {
    tr_log("method not found: %s\n", method);
    return TR_UNDEF;
  }
}

void tr_def(VM, OBJ obj, const char *name, OBJ (*func)(), int argc)
{
  tr_module *mod = (tr_module *) obj;
  tr_method *met = (tr_method *) tr_malloc(sizeof(tr_method));
  
  met->name = tr_malloc(strlen(name)); strcpy(met->name, name);
  met->func = func;
  met->argc = argc;
  
  tr_hash_set(mod->methods, (void *) met->name, (void *) met);
}

OBJ tr_module_new(VM, const char *name)
{
  tr_frame  *f   = CUR_FRAME;
  tr_module *mod = (tr_module *) tr_malloc(sizeof(tr_module));
  
  mod->type     = TR_MODULE;
  mod->name     = tr_malloc(strlen(name)); strcpy(mod->name, name);
  mod->methods  = tr_hash_new();
  mod->imethods = tr_hash_new();
  
  tr_hash_set(f->consts, (void *) mod->name, (void *) mod);
  
  return (OBJ) mod;
}

static OBJ tr_kernel_puts(OBJ txt)
{
  puts((char *) txt);
  
  return TR_NIL;
}

OBJ tr_kernel_init(VM)
{
  OBJ mod = tr_module_new(vm, "Kernel");
  
  tr_def(vm, mod, "puts", tr_kernel_puts, 1);
  
  return mod;
}