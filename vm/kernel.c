#include "tinyrb.h"

static OBJ tr_kernel_puts(VM, OBJ txt)
{
  puts(TR_STR(txt));
  
  return TR_NIL;
}

OBJ tr_kernel_init(VM)
{
  OBJ mod = tr_new(vm, tr_const_get(vm, "Module"));
  tr_const_set(vm, "Kernel", mod);
  
  tr_metadef(vm, mod, "puts", tr_kernel_puts, 1);
  
  return mod;
}