#include "tinyrb.h"

OBJ tr_send(VM, OBJ obj, OBJ message, int argc, OBJ argv[])
{
  tr_module *mod = (tr_module *) obj;
  /* TODO look in imethods */
  OBJ met = tr_hash_get(mod->methods, message);
  
  if (met != TR_NIL) {
    /* TODO handle multiple args */
    return TR_CMETHOD(met)->func(argv[0]);
  } else {
    tr_log("method not found: %s", TR_STR(message));
    return TR_UNDEF;
  }
}

void tr_def(VM, OBJ obj, const char *name, OBJ (*func)(), int argc)
{
  tr_module *mod = (tr_module *) obj;
  tr_method *met = (tr_method *) tr_malloc(sizeof(tr_method));
  
  met->type = TR_METHOD;
  met->name = tr_intern(name);
  met->func = func;
  met->argc = argc;
  
  tr_hash_set(mod->methods, met->name, (OBJ) met);
}

OBJ tr_module_new(VM, const char *name)
{
  tr_frame  *f   = CUR_FRAME;
  tr_module *mod = (tr_module *) tr_malloc(sizeof(tr_module));
  
  mod->type     = TR_MODULE;
  mod->name     = tr_intern(name);
  mod->methods  = tr_hash_new();
  mod->imethods = tr_hash_new();
  
  tr_hash_set(f->consts, mod->name, (OBJ) mod);
  
  return (OBJ) mod;
}

static OBJ tr_kernel_puts(OBJ txt)
{
  puts(TR_STR(txt));
  
  return TR_NIL;
}

void tr_builtins_add(VM)
{
  OBJ mod = tr_module_new(vm, "Kernel");
  
  tr_def(vm, mod, "puts", tr_kernel_puts, 1);
}