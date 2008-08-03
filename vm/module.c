#include "tinyrb.h"

void tr_def(VM, OBJ mod, const char *name, OBJ (*func)(), int argc)
{
  /* TODO */
}

OBJ tr_module(VM, const char *name)
{
  tr_frame *f = CUR_FRAME;
  tr_obj   *mod = (tr_obj *) malloc(sizeof(tr_obj));
  
  /* mod->name = name; */
  mod->type = TR_MODULE;
  
  tr_hash_insert(f->consts, (void *) name, (void *) mod);
  
  return (OBJ) mod;
}

static OBJ tr_kernel_puts(OBJ txt)
{
  puts((char *) txt);
  
  return TR_NIL;
}

OBJ tr_kernel_init(VM)
{
  OBJ mod = tr_module(vm, "Kernel");
  
  tr_def(vm, mod, "puts", tr_kernel_puts, 1);
  
  return mod;
}